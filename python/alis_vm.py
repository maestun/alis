
from doctest import debug_script
import os

import alis_fptrs

from alis_defs import *
from host import HostPlatform, EHostPlatform
from sys_io import SysIO

def dummy():
    pass

kMainScriptID = 0

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
            platform (HostPlatform): Host Platform
            data_path (str): Path to depacked scripts folder
            vram_addr (int): Base RAM address given to us by host system
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

        # load opcodes
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

        # load scripts
        self.platform = platform.value
        self.scripts = {}
        script_files = [f for f in os.listdir(data_path) if (os.path.isfile(os.path.join(data_path, f)) and f.endswith(".alis"))]
        for script_file in script_files:
            # TODO: get org
            script = AlisScript(os.path.join(data_path, script_file), self.platform.is_le)
            self.scripts[script.header.id] = script
            print("Loading script: " + script_file + " (" + hex(script.header.id) + ")")
        
        # vm specs
        packed_main = os.path.join(data_path, "main." + platform.value.extension)
        with open(packed_main, "rb") as f:        
            self.specs = AlisSpecs(f, self.platform.is_le)        
        
        # stack of loaded script ids
        self.ids = [kMainScriptID]
        self.script = self.scripts[kMainScriptID]
        # virtual memory map, used for debug
        self.map = AlisMemoryMap(platform.value.baseram, self.specs, self.script)
        # virtual memory
        self.ram = AlisMemory(platform.value.ram, self.platform.is_le, "alis-vram")
        # virtual accumulator (stack)
        self.acc = []
        # engine variables
        self.vars = AlisVars()
        # system abstraction layer
        self.sys = SysIO(platform, data_path)
        self.op_count = 0
        self.oeval_loop = False

        self.init_entities()
        self.init_sprites()
    # =========================================================================


    # =========================================================================
    def readexec(self, kind: EAlisOpcodeKind):
        # fetch code
        byte = self.script.read()
        opcode = self.opcodes[kind][byte]
        # print(f"{hex(self.script.pc - 1)}: {opcode.name}")
        if kind == EAlisOpcodeKind.OPCODE:
            # TODO: I put an offset to ease debug, remove
            debug_offset = 0x2d290 if self.script.name == "main" else 0x33580 # for logo
            print(f"{hex(self.script.pc - 1 + debug_offset)};{self.op_count};{opcode.name};{hex(opcode.code)};{hex(opcode.addr)}")
            self.op_count += 1
        else:
            print(f"- {opcode.name}")
        opcode.fptr(self)
    # =========================================================================


    # =========================================================================
    def readexec_opcode(self):
        self.readexec(EAlisOpcodeKind.OPCODE)
    # =========================================================================


    # =========================================================================
    def tick(self):
        self.sys.poll_event()
        self.readexec_opcode()
        self.sys.render_frame()
    # =========================================================================

    # =========================================================================
    def run__old(self):
        self.__running = True
        # TODO: restore back self.ui.run(self)
        
        # load main script
        self.script: AlisScript = self.scripts[self.ids[len(self.ids) - 1]]
        while self.__running:
            # TODO: get the correct script

            self.script.is_running = True

            print(f"-- Running: {self.script.name} --")
            self.save_coords()
            self.vars.b_fseq += 1
            while self.script.is_running:
                self.tick()
            
            # we got here cuz of cstop
            if self.script.header.subroutine_offset != 0:
                # execute subroutine, but push pc
                self.vars.b_fseq = 0
                self.acc.append(self.script.pc)
                # in original code the offset starts from script_data + 0x6, but
                # we start from script_code, so remove (HEADER_SZ - 0x6) bytes
                sub_offset = self.script.header.subroutine_offset - (AlisScriptHeader.HEADER_SZ - 0x6)
                self.script.jump(sub_offset, False)
                self.script.is_running = True
                while self.script.is_running:
                    self.tick()
                # restore previous pc
                self.script.pc = self.acc.pop()
            else:
                # TODO: maybe not pop(); instead set previous script
                self.ids.pop()

            # moteur5
            self.update_coords()
            self.script.ctx._0x1_cstart = self.script.ctx._0x2_unknown

    # =========================================================================

    def run(self):
        self.__running = True
        w_offset_atent = 0
        self.script: AlisScript = self.scripts[0]
        self.script.ctx._0x04_unknown_byte = -1 # livemain 0x18ea8
        self.script.ctx._0x1_cstart = 1 # livemain 0x18ea2

        while(self.__running):

            # TODO: le mettre icitte ?
            self.sys.poll_event()

            # __moteur2
            # addr_current_ctx = *(int *)(kAddrAtent + w_offset_atent);

            # TODO: perform context switching here !!
            addr_current_ctx = self.ram.read(self.map.atent + w_offset_atent, 4)
            self.vars.b_fseq = 0
            self.vars.b_fallent = 0
            
            if (self.script.ctx._0x24_scan_clr_bit_7 == 1) and (self.script.ctx._0x24_inter_off_bit_1 == False) and (self.script.header.interrupt_offset != 0):
#         if (// test context: scan/inter/send flags
#             ((*(char *)(addr_current_ctx + kByte_ScanInterFlags_Minus0x24) < 0) &&
#             // test context: bit 2 is INTERRUPT bit (0 == interrupt)
#             ((*(char *)(addr_current_ctx + kByte_ScanInterFlags_Minus0x24) & 2) == 0)) &&
#             // check for interrupt routine offset in current script's header (at offset 0x0a)
#             (*(int *)(*(int *)(addr_current_ctx + kLong_ScriptDataStart_Minus0x14) + 0x0a) != 0)) {
            
                # __before_moteur4:
                self.vars.w_save_rsp = self.script.ctx._0x0a_save_acc_offset
                self.save_coords()
                self.script.is_running = True
                while self.script.is_running:
                    self.readexec_opcode()
                self.update_coords()
#             save_rsp = *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a);
#             savecoord();
#             FUN_READEXEC_OPCODE();
#             updtcoo0();

          # __moteur4:
#         // TODO: what is -4 in ctx ??
#         if (*(char *)(addr_current_ctx + kByte_Minus0x04) != 0) {
            if self.script.ctx._0x04_unknown_byte != 0:
                if self.script.ctx._0x04_unknown_byte < 0:
                    self.script.ctx._0x04_unknown_byte = 1

#             // TODO: what is -1 in ctx ??
#             char pcVar3 = (char *)(addr_current_ctx + kByte_Minus0x01);
                self.script.ctx._0x1_cstart = self.script.ctx._0x1_cstart - 1
#             if (--pcVar3 == 0) {
                if self.script.ctx._0x1_cstart == 0:
                    self.save_coords()
#                 // restore A3 and D4 registers, they're lobal and used by FUN_READEXEC_OPCODE
#                 a3_virtual_pc = *(uint32_t *)(addr_current_ctx + kLong_SaveVirtualProgramCounter_Minus0x08);
                    self.script.pc = self.script.ctx._0x08_save_program_counter
#                 d4_virtual_acc_offset = *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a);
                    self.script.acc = self.script.ctx._0x0a_save_acc_offset
                    
                    self.vars.b_fseq = self.vars.b_fseq + 1
#                 FUN_READEXEC_OPCODE();
                    self.script.is_running = True
                    while self.script.is_running:
                        self.readexec_opcode()
                
#                 // save A3 and D4 registers into context
                    self.script.ctx._0x08_save_program_counter = self.script.pc
                    self.script.ctx._0x0a_save_acc_offset = self.script.acc
#                 *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a) = d4_virtual_acc_offset;
#                 *(uint32_t *)(addr_current_ctx + kLong_SaveVirtualProgramCounter_Minus0x08) = a3_virtual_pc;
                
                    # get start addr of script data, skip 6 bytes to get offset to an optional subroutine
                    if self.script.header.interrupt_offset != 0:
                        # subroutine found
                        self.vars.b_fseq = 0
                        self.vars.w_save_rsp = self.script.ctx._0x0a_save_acc_offset
                        self.script.is_running = True
                        while self.script.is_running:
                            self.readexec_opcode()

                    self.update_coords()
                    self.script.ctx._0x1_cstart = self.script.ctx._0x2_unknown

# __moteur1:
#         // TODO: c'est ici qu'on maj D5 ?
#         w_offset_atent = *(short *)(kAddrAtent + 4 + (int)w_offset_atent);
            w_offset_atent = self.ram.read(self.map.atent + 4 + w_offset_atent, 2)
        
#         // inutile
#         // addr_current_ctx = kAddrAtent;

#         // blit !
#         if (w_offset_atent == 0) {
#             image();
#         }
            if w_offset_atent == 0:
                self.sys.render_frame()
#   }
# }

    # =========================================================================

    def init_entities(self):
        # init atent array
        # contains (max_ent * 6) bytes
        offset = 0
        counter = 0

        while offset <= self.map.debent:
            counter += 6; # sizeof(sEnt)
            offset = self.map.atent + counter
            self.ram.write(offset - 2, counter, 2)
            self.ram.write(offset - 6, 0, 4)


    def init_sprites(self):
        a0 = vm.map.debsprit
        vm.map.basesprit = vm.map.debsprit + 0x8000
        d0 = 0x8000
        vm.vars.w_tvsprite = d0
        vm.ram.write(a0 + d0 + 0xc, 0, 4)
        vm.ram.write(a0 + d0 + 0x16, vm.sys.platform.width - 1, 2)
        vm.ram.write(a0 + d0 + 0x18, vm.sys.platform.height - 1, 2)
        d0 += 0x28
        vm.ram.write(a0 + d0 + 0x1, 0, 1)
        vm.vars.w_backsprite = d0
        d0 += 0x28
        vm.vars.w_texsprite = d0
        vm.ram.write(a0 + d0 + 0xc, 0, 4)
        vm.ram.write(a0 + d0 + 0x0, -2, 1)
        vm.vars.b_mouse_flag = 0
        d0 += 0x28
        vm.vars.w_libsprite = d0

        a1 = a0 + d0 # $28458
        # TODO 29/10 continue here




    def save_coords(self):
        # TODO: offset + A6 / $22690
        self.ram.write(0, self.vars.w_oldcx)
        self.ram.write(4, self.vars.w_oldcy)


    def update_coords(self):
        # TODO: me lazy
        pass

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

# =========================================================================
kProgName = "alis"
kGitHash = os.popen("git rev-parse --short HEAD").read()
kGitTag = os.popen("git tag --points-at HEAD").read()
kGitBranch = os.popen("git rev-parse --abbrev-ref HEAD").read()

ALIS_NAME = "Ishar II / Atari / cracked by Elite"
ALIS_DATA_PATH = "./data/ishar/atari/decrunched"
ALIS_EXE_PATH = "./data/ishar2/atari/auto/ISHAR2OK.PRG"
ALIS_EXE_MD5 = "87471ae02afacf5da303a99ce81ec1cd"

global vm
vm = AlisVM(name=ALIS_NAME, 
            exe_path=ALIS_EXE_PATH,
            exe_md5=ALIS_EXE_MD5,
            exe_addr=0xaa9a,
            data_path=ALIS_DATA_PATH,
            platform=EHostPlatform.Atari,
            opcode_tab_addr=0x12cb6,
            opername_tab_addr=0x12e84,
            storename_tab_addr=0x12f2e,
            addname_tab_addr=0x12f6a)