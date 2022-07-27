# [Debug Addresses]
# Address0=85346
# Mode0=1
# BWR0=0
# MaskW0=0
# MaskR0=0
# Name0=CMOUSE

# Address1=84848
# Mode1=1
# BWR1=0
# MaskW1=0
# MaskR1=0
# Name1=CPICTURE
# Address2=71844
# Mode2=1
# BWR2=1
# MaskW2=0
# MaskR2=0
# Name2=CDEFSC

from enum import Enum, auto
import configparser

class EAlisOpcodeKind(Enum):
    OPCODE = auto()
    OPERNAME = auto()
    STORENAME = auto()
    ADDNAME = auto()


class AlisVM:
    exe_header_sz = 28

    opcode_names = {
        EAlisOpcodeKind.OPCODE: ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", "cpointpix", "cchartmap", "cscsky", "czoom"],
        EAlisOpcodeKind.OPERNAME: ["oimmb", "oimmw", "oimmp", "olocb", "olocw", "olocp", "oloctp", "oloctc", "olocti", "odirb", "odirw", "odirp", "odirtp", "odirtc", "odirti", "omainb", "omainw", "omainp", "omaintp", "omaintc", "omainti", "ohimb", "ohimw", "ohimp", "ohimtp", "ohimtc", "ohimti", "opile", "oeval", "ofin", "cnul", "cnul", "opushacc", "oand", "oor", "oxor", "oeqv", "oegal", "odiff", "oinfeg", "osupeg", "oinf", "osup", "oadd", "osub", "omod", "odiv", "omul", "oneg", "oabs", "ornd", "osgn", "onot", "oinkey", "okeyon", "ojoy", "oprnd", "oscan", "oshiftkey", "ofree", "omodel", "ogetkey", "oleft", "oright", "omid", "olen", "oasc", "ostr", "osadd", "osegal", "osdiff", "osinfeg", "ossupeg", "osinf", "ossup", "ospushacc", "ospile", "oval", "oexistf", "ochr", "ochange", "ocountry", "omip", "ojoykey", "oconfig"],
        EAlisOpcodeKind.STORENAME: ["cnul", "cnul", "cnul", "slocb", "slocw", "slocp", "sloctp", "sloctc", "slocti", "sdirb", "sdirw", "sdirp", "sdirtp", "sdirtc", "sdirti", "smainb", "smainw", "smainp", "smaintp", "smaintc", "smainti", "shimb", "shimw", "shimp", "shimtp", "shimtc", "shimti", "spile", "seval", "ofin"],
        EAlisOpcodeKind.ADDNAME: ["cnul", "cnul", "cnul", "alocb", "alocw", "alocp", "aloctp", "aloctc", "alocti", "adirb", "adirw", "adirp", "adirtp", "adirtc", "adirti", "amainb", "amainw", "amainp", "amaintp", "amaintc", "amainti", "ahimb", "ahimw", "ahimp", "ahimtp", "ahimtc", "ahimti", "spile", "aeval", "ofin"]
    }

    def __init__(self,
                 name,
                 exe_path,
                 exe_md5,
                 base_addr,
                 opcode_tab_addr,
                 opername_tab_addr,
                 storename_tab_addr,
                 addname_tab_addr,
                 ):
        self.name = name
        self.base_addr = base_addr
        self.exe_path = exe_path # todo check md5
        self.exe_md5 = exe_md5
        self.opcode_tab_addrs = {
            EAlisOpcodeKind.OPCODE: opcode_tab_addr,
            EAlisOpcodeKind.OPERNAME: opername_tab_addr,
            EAlisOpcodeKind.STORENAME: storename_tab_addr,
            EAlisOpcodeKind.ADDNAME: addname_tab_addr,
        }
        self.exe_fp = open(exe_path, "r+b")

    
    def get_op_addr(self, kind, name) -> int:
        op_tab = self.opcode_names[kind]
        op_idx = op_tab.index(name)
        op_tab_addr = self.opcode_tab_addrs[kind] 
        offset = op_tab_addr + (op_idx * 2) 
        self.exe_fp.seek(offset - self.base_addr + self.exe_header_sz)

        # read word at (jump_table + offset)
        word = self.exe_fp.read(2)
        op_addr = op_tab_addr + int.from_bytes(word, "big")
        
        print(name + " is located at: " + str(hex(op_addr)))
        return op_addr


INI_BREAKPOINT_STR = """
Address__IDX__=__ADDR__
Mode__IDX__=__MODE__
BWR__IDX__=__BWR__
MaskW__IDX__=__MASK_W__
MaskR__IDX__=__MASK_R__
Name__IDX__=__NAME__
"""

section = "Debug Addresses"
bp_index = 0

def create_bp(alis_vm, kind, name, mode=1, bwr=0, mask_r=0, mask_w=0):
    addr = alis_vm.get_op_addr(kind, name)
    config.set(section, "Address" + str(bp_index), str(addr))
    config.set(section, "Mode" + str(bp_index), str(mode))
    config.set(section, "BWR" + str(bp_index), str(bwr))
    config.set(section, "MaskW" + str(bp_index), str(mask_w))
    config.set(section, "MaskR" + str(bp_index), str(mask_r))
    config.set(section, "Name" + str(bp_index), name)

    bp_index = bp_index + 1



vm = AlisVM(name="Atari Ishar uncracked Daze", 
            exe_path="/home/olivier/dev/perso/atari/alis/data/ishar/atari/auto/start.prg",
            exe_md5="fa11cc8d6166d59edf143e4135ba058c",
            base_addr=0xaa9a,
            opcode_tab_addr=0x10b98,
            opername_tab_addr=0x10d5e,
            storename_tab_addr=0x10e08,
            addname_tab_addr=0x10e44)

config = configparser.ConfigParser()
config.read("/home/olivier/dev/perso/atari/alis/tools/Steem.SSE.4.0.2.Debug.Win32.DD/steem.ini")
config.remove_section(section)

vm.get_op_addr(EAlisOpcodeKind.OPCODE, "cdefsc")
vm.get_op_addr(EAlisOpcodeKind.OPERNAME, "ofin")
vm.get_op_addr(EAlisOpcodeKind.STORENAME, "sdirw")
vm.get_op_addr(EAlisOpcodeKind.ADDNAME, "adirtc")


create_bp(vm, EAlisOpcodeKind.OPCODE, "cdefsc")