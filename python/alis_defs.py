from dataclasses import dataclass
from enum import Enum
import os

# =============================================================================
class EAlisOpcodeKind(str, Enum):
    OPCODE = "opcode"
    OPERAND = "operand"
    STORENAME = "storename"
    ADDNAME = "addname"


# =============================================================================
class AlisOpcode():
    def __init__(self, code: int, fptr, desc, addr):
        self.code = code
        self.fptr = fptr
        self.name = fptr.__name__
        self.desc = desc
        self.addr = addr


# =============================================================================
class AlisSpecs():
    # vm specs are read from packed MAIN script,
    #   located at offset 0x6 and a size of 0x10
    SPECS_OFFSET = 6
    SPECS_SZ = 16
    def __init__(self, f, is_le):
        endian = "little" if is_le else "big"
        _ = f.read(self.SPECS_OFFSET)
        self.max_prog = int.from_bytes(f.read(2), endian)
        self.max_ent = int.from_bytes(f.read(2), endian)
        self.max_ram = int.from_bytes(f.read(4), endian)
        self.debsprit_offset = int.from_bytes(f.read(4), endian)
        self.finsprit_offset = int.from_bytes(f.read(4), endian)


# =============================================================================
class AlisMemory():
    def __init__(self, size: int, is_le: bool, name: str, bytes = []):
        self.endian = "little" if is_le else "big"
        if len(bytes) == 0:
            self.__data = [0] * size
        else:
            self.__data = bytes
        self.__name = name
        print(f"-- {name} created (sz: {hex(size)}) --")


    def read(self, offset: int, size: int, extend: bool = False) -> int:
        """Read some integer data from memory

        Args:
            offset (int): The offset in memory at which reading starts
            size (int): size of integer in bytes (1+)
            extend (int, optional): If non-zero, extend to a byte count. Defaults to 0.

        Returns:
            (int): Signed integer
        """
        val = int.from_bytes(self.__data[offset: offset + size], self.endian)
        if extend:
            if val > (0x80 - 1):
                val = (0x100 - val) * -1
            elif val > (0x8000 - 1):
                val = (0x10000 - val) * -1
        # plural = ("s" if size > 1 else "")
        # print(f"read {size} byte{plural} from {self.__name} at offset {hex(offset)}: {hex(val)}")
        return val
    

    def readp(self, offset: int):
        ret = []
        idx = 0
        c = self.__data[offset]
        while c != 0:
            ret.append(c)
            idx += 1
            c = self.__data[offset + idx]
        ret.append(c)
        return ret


    def write(self, offset: int, value: int, size: int = 1):
        bytes = int.to_bytes(value, size, self.endian)
        src_idx = 0
        for dst_idx in range(offset, offset + size):
            self.__data[dst_idx] = bytes[src_idx]
            src_idx += 1
        # plural = ("s" if size > 1 else "")
        # print(f"wrote {size} byte{plural} ({hex(value)}) into {self.__name} at offset {hex(offset)}")


    def writep(self, offset, value):
        src_idx = 0
        for dst_idx in range(offset, offset + len(value)):
            self.__data[dst_idx] = value[src_idx]
            src_idx += 1


# =============================================================================
class AlisScriptHeader():
    HEADER_SZ = 24
    def __init__(self, f, is_le):
        endian = "little" if is_le else "big"
        self.id = int.from_bytes(f.read(2), endian)
        # seems to be always 0x1700 on atari, copied at (vram - $2e)
        self.w_0x1700 = int.from_bytes(f.read(2), endian)
        # where does code start from (script header + id) ? on atari, always $16
        self.code_loc_offset = int.from_bytes(f.read(2), endian)
        # some scripts have a "sub-script" into them: this is the offset to their code location
        self.subroutine_offset = int.from_bytes(f.read(4), endian)
        # TODO: ???
        self.interrupt_offset = int.from_bytes(f.read(4), endian)
        # TODO: ???
        self.dw_unknown4 = int.from_bytes(f.read(4), endian)
        # almost always 0x20 (is 0xa for "message*" and 0x4 for "objet")
        self.w_unknown5 = int.from_bytes(f.read(2), endian)
        # number of bytes to alloc fo this script
        self.vram_alloc_sz = int.from_bytes(f.read(2), endian)
        # almost always 0x20 (is 0x4 for "message*" and "objet")
        self.w_unknown7 = int.from_bytes(f.read(2), endian)


# =============================================================================
# TODO: inherit from AlisMemory, to get std read and write methods
class AlisScript():
    def __init__(self, path: str, is_le: bool):
        with open(path, "rb") as f:
            self.name = os.path.basename(path).split(".")[0]
            # read HEADER_SZ bytes
            self.header = AlisScriptHeader(f, is_le)
            # remaining bytes are script code & data
            self.size = os.stat(path).st_size - AlisScriptHeader.HEADER_SZ
            self.code = AlisMemory(self.size, is_le, self.name + "-code", f.read())
            f.close()
            
            # is alloc virtual ram needed ?
            # ...NOPE :)
            # self.vram = AlisMemory(self.header.vram_alloc_sz, is_le, self.name + "-vram")

            # set program counter to 1st script CODE byte, not DATA !
            self.pc = 0 # register A3
            # I've set one virtual accumulator per script, maybe a global one is sufficient
            # ...YEP :)
            # self.acc = 0 # register D4

            self.is_running = False

            # each script has its own context
            self.ctx = AlisScriptContext()


    def read(self, size: int = 1, extend: int = 0):
        if size > 0:
            val = self.code.read(self.pc, size, extend)
            self.pc += size
            return val
        else:
            val = self.code.readp(self.pc)
            self.pc += len(val)
            return val


    # def vread(self, offset: int, size: int = 1, extend: bool = False):
    #     return self.vram.read(offset, size, extend)

    # def vwrite(self, offset: int, value: int, size: int = 1):
    #     self.vram.write(offset, value, size)


    def jump(self, offset, is_offset = True):
        """Relocates or moves this script's Program Counter

        Args:
            offset (int): jump value
            is_offset (bool, optional): if true, value is added to PC, else PC is relocated. Defaults to True.
        """
        self.pc = offset + (self.pc if is_offset else 0)


    def live(self, vm, is_main = False):
        if is_main == False:
    #                                  *******************************************************
    #                      *                      FUNCTION                       *
    #                      *******************************************************
    #                      undefined __stdcall liveprog(void)
    #        undefined       D0b:1        <RETURN>
    #                      liveprog                                  XREF[1]:   OPCODE_CWLIVE_0x3c:0001708e(
    #   0001d830 20 79 00      movea.l   (addr_atent).l,A0
    #            01 e5 54
    #   0001d836 30 30 50      move.w    (0x4,A0,D5w*0x1),D0w
    #            04
            w_offset_atent = d5
            d0 = vm.ram.read(vm.map.atent + 4 + w_offset_atent, 2)
    #   0001d83a 32 39 00      move.w    (dernent).l,D1w
    #            01 e5 64

    #   0001d840 31 81 50      move.w    D1w,(0x4,A0,D5w*0x1)
    #            04
    #   0001d844 33 f0 10      move.w    (0x4,A0,D1w*0x1),(dernent).l
    #            04 00 01 
    #            e5 64
    #   0001d84c 31 80 10      move.w    D0w,(0x4,A0,D1w*0x1)
    #            04
    #   0001d850 52 79 00      addq.w    #0x1,(nbent).l
    #            01 e5 62
    #   0001d856 30 39 00      move.w    (nbent).l,D0w
    #            01 e5 62
    #   0001d85c b0 79 00      cmp.w     (maxent).l,D0w
    #            01 e5 60
    #   0001d862 6e 00 00      bgt.w     livee
    #            ee



# =============================================================================
class AlisMemoryMap():
    def __init__(self, baseram, specs: AlisSpecs, main: AlisScript):
        self.basemem = baseram
        self.atprog = self.dernprog = self.basemem
        self.atent = self.basemem + (4 * specs.max_prog) # sizeof(u32)
        self.debent = self.atent + (6 * specs.max_ent) # sizeof(u32) + sizeof(u16)
        self.basemain = self.debent + main.header.w_unknown5 + main.header.w_unknown7 + 52 # 52: hardcoded, sizeof ctx ??
        self.finent = self.basemain + main.header.vram_alloc_sz
        self.debsprit = ((self.debent + specs.debsprit_offset) | 0x0f) + 1
        self.finsprit = self.debsprit + ((specs.finsprit_offset + 3) * 40)
        self.debprog = self.finsprit
        self.finprog = self.debprog
        self.basesprit = self.debsprit + 0x8000 # hardcoded

        print("-- Memory Map --")
        print(f"basemem: {hex(self.basemem)}")
        print(f"atprog: {hex(self.atprog)}")
        print(f"atent: {hex(self.atent)}")
        print(f"debent: {hex(self.debent)}")
        print(f"basemain: {hex(self.basemain)}")
        print(f"finent: {hex(self.finent)}")
        print(f"debsprit: {hex(self.debsprit)}")
        print(f"finsprit: {hex(self.finsprit)}")
        print(f"debprog: {hex(self.debprog)}")
        print(f"finprog: {hex(self.finprog)}")
        print(f"basesprit: {hex(self.basesprit)}")
        

# =============================================================================
@dataclass
class AlisVars():
    d6 = 0
    d7 = 0
    sd6 = [0] * 256
    sd7 = [0] * 256
    oldsd7 = [0] * 256

    b_automode = 0
    b_fallent = 0
    b_fseq = 0
    b_timing = 0
    b_tvmode = 0
    b_flaginvx = 0
    b_flagmain = 0
    b_fmuldes = 0
    b_numelem = 0
    b_mouse_flag = 0

    w_cx = 0
    w_cy = 0
    w_cz = 0
    w_crnd = 0
    w_fswitch = 0
    w_depx = 0
    w_depy = 0
    w_depz = 0
    w_oldcx = 0
    w_oldcy = 0
    w_oldcz = 0
    w_save_rsp = 0
    w_libsprite = 0
    w_tvsprite = 0
    w_texsprite = 0
    w_backsprite = 0


# =============================================================================
# VM context for each script, located at A6 / $22690 for main script
# Each script context address is located at the address "atent" in AlisMemoryMap
@dataclass
class AlisScriptContext():
   _0x1_cstart = 0
   _0x2_unknown = 0
   _0x3_invx = 0
   _0x04_unknown_byte = 0
   _0x08_save_program_counter = 0
   _0x0a_save_acc_offset = 0
   _0x1c_scan_clr = 0
   _0x1e_scan_clr = 0
   _0x22_cscreen = 0
   _0x24_scan_off_bit_0 = False
   _0x24_scan_clr_bit_7 = False
   _0x24_inter_off_bit_1 = False

