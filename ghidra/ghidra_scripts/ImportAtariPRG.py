# This script imports an Atari TOS program into Ghidra creating TEXT, DATA, BSS sections
#@author Christian Zietz
#@category Import
#@keybinding 
#@menupath 
#@toolbar 

# Copyright (c) 2019 Christian Zietz
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import struct
import jarray

# Arbitrary address to relocate program to
reloc_addr = 0xaa9a

# Ask user for the file to import
file = askFile("Please specify a file to import", "Import")

# Import as binary file for 68k CPU since there is no builtin support for TOS program file format
lang = getDefaultLanguage(ghidra.program.model.lang.Processor.findOrPossiblyCreateProcessor("68000"))
comp = lang.getDefaultCompilerSpec()
program = importFileAsBinary(file, lang, comp)
flat = ghidra.program.flatapi.FlatProgramAPI(program)

# Initialize some variables
txn = program.startTransaction("Import program")
mem = program.getMemory()
start = mem.getMinAddress()

# Check if the extension is CPX (control panel file)
if file.toString()[-4:].upper() == ".CPX":
    # remove CPX header
    prg_start = start.add(512)
    mem.split(mem.getBlocks()[0], prg_start)
    mem.removeBlock(mem.getBlocks()[0], ghidra.util.task.TaskMonitor.DUMMY)
    # move actual program start to 0
    mem.moveBlock(mem.getBlocks()[0], start, ghidra.util.task.TaskMonitor.DUMMY)

# Check for "magic" number in header
start = mem.getMinAddress()
magic = mem.getShort(start)
if magic != 0x601a:
    raise Exception("Not a TOS program!")

# Data from PRG header
len_text = mem.getInt(start.add(0x2))
len_data = mem.getInt(start.add(0x6))
len_bss  = mem.getInt(start.add(0xa))
len_sym  = mem.getInt(start.add(0xe))
has_relo = (mem.getShort(start.add(0x1a)) == 0)

# Keep symbol table for later use
if len_sym > 0:
    sym_table = jarray.zeros(len_sym, "b")
    mem.getBytes(start.add(0x1c+len_text+len_data), sym_table)
    sym_table = bytearray(sym_table) # to native Python type

if has_relo:
    # Relocate program
    prg = start.add(0x1c)
    ptr = start.add(0x1c+len_text+len_data+len_sym) # start of relocation table
    rea = mem.getInt(ptr) # first address to relocate
    ptr = ptr.add(4)
    if rea != 0:
        # print("Relocating %x (%08x => %08x)" % (rea, mem.getInt(prg.add(rea)), mem.getInt(prg.add(rea))+reloc_addr))
        mem.setInt(prg.add(rea), mem.getInt(prg.add(rea)) + reloc_addr)
        while True:
            offs = mem.getByte(ptr)
            if offs<0: # byte is *signed* in Java/Jython
                offs=256+offs
            ptr = ptr.add(1)
            if offs == 0: # end of table
                break
            if offs == 1: # advance by 254
                rea = rea + 254
                continue
            rea = rea + offs
            # print("Relocating %x (%08x => %08x)" % (rea, mem.getInt(prg.add(rea)), mem.getInt(prg.add(rea))+reloc_addr))
            mem.setInt(prg.add(rea), mem.getInt(prg.add(rea)) + reloc_addr)
            # when we are already in the data segment, create a dword here
            if (rea >= len_text):
                data = flat.createDWord(prg.add(rea)) # FIXME: should be pointer

sym_format = -1
#
# check for GNU binutils aexec header
#
if len_text > 0xe4 and mem.getInt(start.add(18)) == 0x4d694e54 and ((mem.getInt(start.add(28)) == 0x283a001a and mem.getInt(start.add(32)) == 0x4efb48fa) or (mem.getInt(start.add(28)) == 0x203a001a and mem.getInt(start.add(32)) == 0x4efb08fa)) and (mem.getShort(start.add(38)) == 0x0108 or mem.getShort(start.add(38)) == 0x010b):
    a_syms = mem.getInt(start.add(52))
    sym_format = mem.getInt(start.add(80))
elif len_sym > 0:
    sym_format = 1
# Primitive plausibility check: a DRI/GST symbol table size always is a multiple of 14
if len_sym == 0 or (sym_format == 1 and (len_sym % struct.calcsize(">8sHL")) != 0):
    sym_format = -1

# Header Block, split off actual TEXT, DATA sections
bl_hdr = mem.getBlocks()[0] # only one block exists right now
bl_hdr.setName("Program Header")
mem.split(bl_hdr, start.add(0x1c))

# Create TEXT and DATA sections at relocated address
bl_text = mem.getBlocks()[-1] # newly split block
bl_text.setName("TEXT")
text_start = start.add(reloc_addr)
data_start = text_start.add(len_text)
bss_start =  data_start.add(len_data)
mem.moveBlock(bl_text, text_start, ghidra.util.task.TaskMonitor.DUMMY)

# Split off data block
if len_data!=0:
    mem.split(bl_text, data_start)
    bl_data = mem.getBlocks()[-1] # newly split block
    bl_data.setName("DATA")
else:
    bl_data = bl_text

# Delete everything after end of DATA, create new empty BSS block instead
if len_bss!=0:
    try:
        mem.split(bl_data, bss_start)
        bl_bss = mem.getBlocks()[-1] # new block
        mem.removeBlock(bl_bss, ghidra.util.task.TaskMonitor.DUMMY)
    except:
        # fails if there is nothing (no relocation table or symbol table) after data section
        pass
    bl_bss = mem.createUninitializedBlock("BSS", bss_start, len_bss, False)
    bl_bss.setRead(True)
    bl_bss.setWrite(True)

# Change the name of the fragment created when program was loaded
frag = program.getTreeManager().getFragment(ghidra.program.database.module.TreeManager.DEFAULT_TREE_NAME, start)
frag.setName("Header, TEXT, DATA")

# Import symbol table if the user wants to
if sym_format >= 0:
    if askYesNo("Import", "Import symbol table?") != True:
        sym_format = -1

# because Ghidra's Python doesn't have this
def my_iter_unpack(fmt, buffer):
    fmtsize = struct.calcsize(fmt)
    for i in range(0, len(buffer), fmtsize):
        yield struct.unpack_from(fmt, buffer, i)

# get a zero terminated string from a buffer, starting at pos
def get_string(s, pos):
   term = s.find(b"\0", pos + 1)
   if term != -1:
      return s[pos:term].decode("ascii")
   else:
      return s[pos:].decode("ascii")

# parse symbol table into list
symbols = []
if sym_format == 1:
    s_ptr = 0
    while s_ptr < len_sym:
        s_name, s_id, s_addr = struct.unpack_from(">8sHL", bytes(sym_table), s_ptr)
        s_ptr += struct.calcsize(">8sHL")
        if s_addr == 0xFFFFFFFF:
            continue

        # extended GST format: read name from next slot
        if s_id & 0x48 != 0:
            s_name = s_name + struct.unpack_from(">14s",  bytes(sym_table), s_ptr)[0]
            s_ptr += struct.calcsize(">14s")

        s_name = s_name.rstrip("\0")

        # skip symbols containing file names
        if s_name[-2:] == ".o" or s_name[-2:] == ".a" or s_name[0] == "/":
            continue

        s_type = s_id & 0xf00
        if s_type == 0x200: # TEXT
            symbols += [{"name":s_name,"addr":text_start.add(s_addr)}]
        elif s_type == 0x400: # DATA
            symbols += [{"name":s_name,"addr":data_start.add(s_addr)}]
        elif s_type == 0x100: # BSS
            symbols += [{"name":s_name,"addr":bss_start.add(s_addr)}]
        else: # unsupported type
            pass
elif sym_format == 0:
    string_size = struct.unpack_from(">l", bytes(sym_table[a_syms:a_syms+4]))[0]
    #print("a_sym size: %x" % (a_syms))
    #print("sym size: %x" % (len_sym))
    #print("string size: %x %x" % (string_size, len_sym - a_syms))
    string_table = bytes(sym_table[a_syms:a_syms+4+string_size])
    # fictious section for external references
    ext_size = len_bss
    sym_table = bytes(sym_table[0:a_syms])
    for (n_strx, n_type, n_other, n_desc, n_value) in my_iter_unpack(">lBbhL", sym_table):
        if n_strx == 0:
            continue
        n_name = get_string(string_table, n_strx)
        #print("s: %08x %s" % (n_strx, n_name))

        # skip symbols containing file names
        if n_name[-2:] == ".o" or n_name[-2:] == ".a" or n_name.find("/") >= 0:
            continue

        n_type &= 0x1f # mask for type
        if n_type == 0x00 or n_type == 0x01: # undefined or common
            if n_value == 0: # undefined=external symbol
                # create external symbol arbitrarily sized 4 bytes
                # n_addr = bss_start.add(ext_size)
                n_addr = None
                ext_size += 4
            else: # common symbol = uninitialized variable with given length
                # add to bss
                # n_addr = bss_start.add(ext_size)
                n_addr = None
                ext_size += n_value
        elif n_type == 0x02 or n_type == 0x03 or n_type == 0x0e: # absolute
            # n_addr = n_value # TODO: create absolute symbol
            n_addr = None
        elif n_type == 0x04 or n_type == 0x05 or n_type == 0x0f: # text
            n_addr = text_start.add(n_value)
            #print("%08x T %s" % (n_addr.offset, n_name))
        elif n_type == 0x06 or n_type == 0x07 or n_type == 0x10: # data
            n_addr = data_start.add(n_value - len_text)
            #print("%08x D %s" % (n_addr.offset, n_name))
        elif n_type == 0x08 or n_type == 0x09 or n_type == 0x11: # bss
            n_addr = bss_start.add(n_value - len_text - len_data)
            #print("%08x B %s" % (n_addr.offset, n_name))
        elif n_type == 0x1c or n_type == 0x1d:                   # N_SETV; ignored
            n_addr = None
        else:
            print("Unknown symbol type %d" % n_type)
            n_addr = None
        if n_addr != None:
            symbols += [{"name":n_name,"addr":n_addr}]

# add symbols
for sym in symbols:
    # skip local labels
    if sym["name"][0:2] != ".L":
        flat.createLabel(sym["addr"], sym["name"], False)

# Add some labels
flat.createDwords(start.add(0x2), 6)
flat.createLabel(start.add(0x2), "TEXT_LEN", True)
flat.createLabel(start.add(0x6), "DATA_LEN", True)
flat.createLabel(start.add(0xa), "BSS_LEN", True)
flat.createLabel(start.add(0xe), "SYM_LEN", True)
flat.createLabel(start.add(0x12), "RESERVED", True)
flat.createLabel(start.add(0x16), "PRGFLAGS", True)
flat.createWord(start.add(0x1A))
flat.createLabel(start.add(0x1A), "ABSOLUTE", True)
flat.createLabel(text_start, "ENTRY_POINT", True)

# Run disassembler on entry point
flat.disassemble(text_start)

# Open for user to see and to start analyzing
program.endTransaction(txn, True)
openProgram(program)
