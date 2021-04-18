# This GHIDRA SCRIPT creates functions in an ALIS Atari program
#@author maestun
#@category ALIS
#@keybinding 
#@menupath 
#@toolbar 

import struct
import jarray

#############################################
# CHANGE DATA HERE DEPENDING ON VM EXECUTABLE
reloc_addr = 0xaa9a

# ISHAR 1 / CR ELITE
# alis_jtab_opcodes = 0x10d22
# alis_jtab_opcodes_len = 454
# alis_jtab_opernames = 0x10ee8
# alis_jtab_opernames_len = 
# alis_jtab_storenames = 0x
# alis_jtab_storenames_len = 
# alis_jtab_addnames = 0x
# alis_jtab_addnames_len = 


# ISHAR 2 / CR ELITE / ATARI ST
alis_jtab_opcodes_addr = 0x12cb6
alis_jtab_opcodes_len = 462

alis_jtab_opernames_addr = 0x12e84
alis_jtab_opernames_len = 170

alis_jtab_storenames_addr = 0x12f2e
alis_jtab_storenames_len = 60

alis_jtab_addnames_addr = 0x12f6a
alis_jtab_addnames_len = 60
#############################################


# ISHAR 2 / FALCON
alis_jtab_opcodes_addr = 0x148ac
alis_jtab_opcodes_len = 462

alis_jtab_opernames_addr = 0x14a7a
alis_jtab_opernames_len = 170

alis_jtab_storenames_addr = 0x14b24
alis_jtab_storenames_len = 60

alis_jtab_addnames_addr = 0x14b60
alis_jtab_addnames_len = 60
#############################################


# COMMON TO ALL VMs ?
alis_jtab_opcodes_names = ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", "cpointpix", "cchartmap", "cscsky", "czoom"]
alis_jtab_opernames_names = ["oimmb", "oimmw", "oimmp", "olocb", "olocw", "olocp", "oloctp", "oloctc", "olocti", "odirb", "odirw", "odirp", "odirtp", "odirtc", "odirti", "omainb", "omainw", "omainp", "omaintp", "omaintc", "omainti", "ohimb", "ohimw", "ohimp", "ohimtp", "ohimtc", "ohimti", "opile", "oeval", "ofin", "cnul", "cnul", "opushacc", "oand", "oor", "oxor", "oeqv", "oegal", "odiff", "oinfeg", "osupeg", "oinf", "osup", "oadd", "osub", "omod", "odiv", "omul", "oneg", "oabs", "ornd", "osgn", "onot", "oinkey", "okeyon", "ojoy", "oprnd", "oscan", "oshiftkey", "ofree", "omodel", "ogetkey", "oleft", "oright", "omid", "olen", "oasc", "ostr", "osadd", "osegal", "osdiff", "osinfeg", "ossupeg", "osinf", "ossup", "ospushacc", "ospile", "oval", "oexistf", "ochr", "ochange", "ocountry", "omip", "ojoykey", "oconfig"]
alis_jtab_storenames_names = ["cnul", "cnul", "cnul", "slocb", "slocw", "slocp", "sloctp", "sloctc", "slocti", "sdirb", "sdirw", "sdirp", "sdirtp", "sdirtc", "sdirti", "smainb", "smainw", "smainp", "smaintp", "smaintc", "smainti", "shimb", "shimw", "shimp", "shimtp", "shimtc", "shimti", "spile", "seval", "ofin"]
alis_jtab_addnames_names = ["cnul", "cnul", "cnul", "alocb", "alocw", "alocp", "aloctp", "aloctc", "alocti", "adirb", "adirw", "adirp", "adirtp", "adirtc", "adirti", "amainb", "amainw", "amainp", "amaintp", "amaintc", "amainti", "ahimb", "ahimw", "ahimp", "ahimtp", "ahimtc", "ahimti", "spile", "aeval", "ofin"]
#############################################

state = getState()
program = state.getCurrentProgram()
mem = program.getMemory()
start = mem.getMinAddress()
flat = ghidra.program.flatapi.FlatProgramAPI(program)


def createALISFuntions(alis_jtab_addr, alis_jtab_len, alis_jtab_names, alis_prefix):
    i = 0
    createLabel(start.getNewAddress(alis_jtab_addr), "JTAB_" + alis_prefix + "S", True)
    for code in range(0, alis_jtab_len / 2):
        print(alis_prefix +": " + hex(code))
        offset_addr = start.getNewAddress(alis_jtab_addr + (code * 2))
        print("Offset addr: " + offset_addr.toString())
        offset = mem.getShort(offset_addr, True)
        print("Offset value: " + hex(offset))
        function_addr = start.getNewAddress(alis_jtab_addr + offset)
        print(alis_prefix + " addr: " + function_addr.toString())
        function_name = alis_prefix + "_" + alis_jtab_names[code].upper() + "_" + hex(code)
        print("Creating function: " + function_name + " at addr $" + function_addr.toString())
        print("----------------------------------")
        createFunction(function_addr, function_name)
        flat.disassemble(function_addr)
        i = i + 1
    print("----------------------------------")
    print("Created " + str(i) + " " + alis_prefix + " functions")
    print("----------------------------------")


createALISFuntions(alis_jtab_opcodes_addr, alis_jtab_opcodes_len, alis_jtab_opcodes_names, "OPCODE")
createALISFuntions(alis_jtab_opernames_addr, alis_jtab_opernames_len, alis_jtab_opernames_names, "OPERNAME")
createALISFuntions(alis_jtab_storenames_addr, alis_jtab_storenames_len, alis_jtab_storenames_names, "STORENAME")
createALISFuntions(alis_jtab_addnames_addr, alis_jtab_addnames_len, alis_jtab_addnames_names, "ADDNAME")
