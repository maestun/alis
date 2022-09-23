
import os

from alis_defs import AlisMemory, AlisOpcode, AlisScript, AlisVars, EAlisOpcodeKind
import alis_fptrs
from host import HostPlatform
from debug_ui import DebugUI
from sys_io import SysIO

def dummy():
    pass

# =============================================================================
class AlisVM():
    __exe_header_sz = 28
    __opcode_names = {
        EAlisOpcodeKind.OPCODE: ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", "cpointpix", "cchartmap", "cscsky", "czoom"],
        EAlisOpcodeKind.OPERAND: ["oimmb", "", "oimmw", "", "oimmp", "", "olocb", "", "olocw", "", "olocp", "", "oloctp", "", "oloctc", "", "olocti", "", "odirb", "", "odirw", "", "odirp", "", "odirtp", "", "odirtc", "", "odirti", "", "omainb", "", "omainw", "", "omainp", "", "omaintp", "", "omaintc", "", "omainti", "", "ohimb", "", "ohimw", "", "ohimp", "", "ohimtp", "", "ohimtc", "", "ohimti", "", "opile", "", "oeval", "", "ofin", "", "cnul", "", "cnul", "", "opushacc", "", "oand", "", "oor", "", "oxor", "", "oeqv", "", "oegal", "", "odiff", "", "oinfeg", "", "osupeg", "", "oinf", "", "osup", "", "oadd", "", "osub", "", "omod", "", "odiv", "", "omul", "", "oneg", "", "oabs", "", "ornd", "", "osgn", "", "onot", "", "oinkey", "", "okeyon", "", "ojoy", "", "oprnd", "", "oscan", "", "oshiftkey", "", "ofree", "", "omodel", "", "ogetkey", "", "oleft", "", "oright", "", "omid", "", "olen", "", "oasc", "", "ostr", "", "osadd", "", "osegal", "", "osdiff", "", "osinfeg", "", "ossupeg", "", "osinf", "", "ossup", "", "ospushacc", "", "ospile", "", "oval", "", "oexistf", "", "ochr", "", "ochange", "", "ocountry", "", "omip", "", "ojoykey", "", "oconfig", "", "ofin"],
        EAlisOpcodeKind.STORENAME: ["cnul", "", "cnul", "", "cnul", "", "slocb", "", "slocw", "", "slocp", "", "sloctp", "", "sloctc", "", "slocti", "", "sdirb", "", "sdirw", "", "sdirp", "", "sdirtp", "", "sdirtc", "", "sdirti", "", "smainb", "", "smainw", "", "smainp", "", "smaintp", "", "smaintc", "", "smainti", "", "shimb", "", "shimw", "", "shimp", "", "shimtp", "", "shimtc", "", "shimti", "", "spile", "", "seval", "", "ofin"],
        EAlisOpcodeKind.ADDNAME: ["cnul", "", "cnul", "", "cnul", "", "alocb", "", "alocw", "", "alocp", "", "aloctp", "", "aloctc", "", "alocti", "", "adirb", "", "adirw", "", "adirp", "", "adirtp", "", "adirtc", "", "adirti", "", "amainb", "", "amainw", "", "amainp", "", "amaintp", "", "amaintc", "", "amainti", "", "ahimb", "", "ahimw", "", "ahimp", "", "ahimtp", "", "ahimtc", "", "ahimti", "", "spile", "", "aeval", "", "ofin"]
    }
    __opcode_addrs = {
        EAlisOpcodeKind.OPCODE: [],
        EAlisOpcodeKind.OPERAND: [],
        EAlisOpcodeKind.STORENAME: [],
        EAlisOpcodeKind.ADDNAME: []
    }
    opcodes = {
        EAlisOpcodeKind.OPCODE: [],
        EAlisOpcodeKind.OPERAND: [],
        EAlisOpcodeKind.STORENAME: [],
        EAlisOpcodeKind.ADDNAME: []
    }

    # =========================================================================
    def __init__(self,
                 name: str,
                 exe_path: str,
                 exe_md5: str,
                 exe_addr: int,
                 platform: HostPlatform,
                 data_path: str,
                 vram_addr: int,
                 opcode_tab_addr: int,
                 opername_tab_addr: int,
                 storename_tab_addr: int,
                 addname_tab_addr: int,
                 fake_opcodes: bool = False
                 ):
        """Creates an ALIS Virtual Machine from a given executable

        Args:
            name (str): name of the game
            exe_path (str): path to the game's executable (unpacked)
            exe_md5 (str): executable's MD5 sum
            exe_addr (int): Address at which the executable was loaded in emulator
            opcode_tab_addr (int): Address of OPCODE jump table in emulator
            opername_tab_addr (int): Address of OPERAND jump table in emulator
            storename_tab_addr (int): Address of STORE jump table in emulator
            addname_tab_addr (int): Address of ADD jump table in emulator
            fake_opcodes (bool): Set to True to generate a fake opcode table, useful for code generation
        """
        self.name = name
        self.exe_addr = exe_addr
        self.exe_path = exe_path # TODO: check md5
        self.exe_md5 = exe_md5
        self.opcode_tab_addrs = {
            EAlisOpcodeKind.OPCODE: opcode_tab_addr,
            EAlisOpcodeKind.OPERAND: opername_tab_addr,
            EAlisOpcodeKind.STORENAME: storename_tab_addr,
            EAlisOpcodeKind.ADDNAME: addname_tab_addr,
        }
        self.exe_fp = open(exe_path, "r+b")

        for kind, addrs in self.__opcode_addrs.items():
            code = 0
            for name in self.__opcode_names[kind]:
                addr = self.compute_op_addr(kind, name)
                addrs.append(addr)
                if fake_opcodes:
                    self.opcodes[kind].append( AlisOpcode(code, dummy, name, addr) )
                else:
                    self.opcodes[kind].append( AlisOpcode(code, getattr(alis_fptrs, name), name, addr) if name != "" else "")
                code += 1
        
        self.platform: HostPlatform = platform.value
        self.ram = AlisMemory(vram_addr, 256 * 1024, self.platform.is_le, "ALIS-vram")
        self.acc = []
        self.vars = AlisVars()
        self.sys = SysIO()
        self.ui = DebugUI()
        self.op_count = 0
        self.oeval_loop = False

        # load scripts
        self.scripts = {}
        script_files = [f for f in os.listdir(data_path) if (os.path.isfile(os.path.join(data_path, f)) and f.endswith(".alis"))]
        for script_file in script_files:
            # TODO: get org
            script = AlisScript(os.path.join(data_path, script_file), self.platform.is_le)
            self.scripts[script.header.id] = script
            print("Loading script: " + script_file + " (" + hex(script.header.id) + ")")
    # =========================================================================


    # =========================================================================
    def readexec(self, kind: EAlisOpcodeKind):
        # fetch code
        byte = self.script.cread(1, False)
        opcode = self.opcodes[kind][byte]
        print(f"{hex(self.script.pc - 1)}: {opcode.name}")
        opcode.fptr(self)
    # =========================================================================


    # =========================================================================
    def readexec_opcode(self):
        self.readexec(EAlisOpcodeKind.OPCODE)
    # =========================================================================


    # =========================================================================
    def tick(self):
        self.sys.poll_event()
        self.op_count += 1
        print(f"op_count: {self.op_count} - ", end="")
        self.readexec_opcode()
        self.sys.render_frame()
    # =========================================================================

    # =========================================================================
    def run(self):
        self.script = self.scripts[0]
        self.script.is_running = True
        self.__running = True
        self.ui.run(self)

        # print(f"-- {self.script.name} --")
        # while True:
        #     self.sys.poll_event()
        #     self.readexec_opcode()
        #     self.sys.render_frame()
    # =========================================================================

    def get_op_addr(self, kind, name) -> int:
        op_tab = self.__opcode_names[kind]
        op_idx = op_tab.index(name)
        return self.__opcode_addrs[kind][op_idx]


    def get_op_name(self, kind, addr) -> int:
        op_tab = self.__opcode_addrs[kind]
        op_idx = op_tab.index(addr)
        return self.__opcode_names[kind][op_idx]


    def compute_op_addr(self, kind, name) -> int:
        op_tab = self.__opcode_names[kind]
        op_idx = op_tab.index(name)
        op_tab_addr = self.opcode_tab_addrs[kind] 
        offset = op_tab_addr + (op_idx * 2) 
        self.exe_fp.seek(offset - self.exe_addr + self.__exe_header_sz)

        # read word at (jump_table + offset)
        word = self.exe_fp.read(2)
        op_addr = op_tab_addr + int.from_bytes(word, "big")
        return op_addr

