
from enum import Enum, IntEnum
import configparser
import os


# =============================================================================
class EAlisOpcodeKind(str, Enum):
    OPCODE = "opcode"
    OPERNAME = "opername"
    STORENAME = "storename"
    ADDNAME = "addname"


# =============================================================================
class ESteemBreakMode(IntEnum):
    NONE = 0
    GLOBAL = 1
    BREAK = 2
    # in steem select menu > Log2 > log system active
    LOG = 3


# =============================================================================
class AlisVM:
    exe_header_sz = 28

    opcode_names = {
        EAlisOpcodeKind.OPCODE: ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", "cpointpix", "cchartmap", "cscsky", "czoom"],
        EAlisOpcodeKind.OPERNAME: ["oimmb", "oimmw", "oimmp", "olocb", "olocw", "olocp", "oloctp", "oloctc", "olocti", "odirb", "odirw", "odirp", "odirtp", "odirtc", "odirti", "omainb", "omainw", "omainp", "omaintp", "omaintc", "omainti", "ohimb", "ohimw", "ohimp", "ohimtp", "ohimtc", "ohimti", "opile", "oeval", "ofin", "cnul", "cnul", "opushacc", "oand", "oor", "oxor", "oeqv", "oegal", "odiff", "oinfeg", "osupeg", "oinf", "osup", "oadd", "osub", "omod", "odiv", "omul", "oneg", "oabs", "ornd", "osgn", "onot", "oinkey", "okeyon", "ojoy", "oprnd", "oscan", "oshiftkey", "ofree", "omodel", "ogetkey", "oleft", "oright", "omid", "olen", "oasc", "ostr", "osadd", "osegal", "osdiff", "osinfeg", "ossupeg", "osinf", "ossup", "ospushacc", "ospile", "oval", "oexistf", "ochr", "ochange", "ocountry", "omip", "ojoykey", "oconfig"],
        EAlisOpcodeKind.STORENAME: ["cnul", "cnul", "cnul", "slocb", "slocw", "slocp", "sloctp", "sloctc", "slocti", "sdirb", "sdirw", "sdirp", "sdirtp", "sdirtc", "sdirti", "smainb", "smainw", "smainp", "smaintp", "smaintc", "smainti", "shimb", "shimw", "shimp", "shimtp", "shimtc", "shimti", "spile", "seval", "ofin"],
        EAlisOpcodeKind.ADDNAME: ["cnul", "cnul", "cnul", "alocb", "alocw", "alocp", "aloctp", "aloctc", "alocti", "adirb", "adirw", "adirp", "adirtp", "adirtc", "adirti", "amainb", "amainw", "amainp", "amaintp", "amaintc", "amainti", "ahimb", "ahimw", "ahimp", "ahimtp", "ahimtc", "ahimti", "spile", "aeval", "ofin"]
    }

    opcode_addrs = {
        EAlisOpcodeKind.OPCODE: [],
        EAlisOpcodeKind.OPERNAME: [],
        EAlisOpcodeKind.STORENAME: [],
        EAlisOpcodeKind.ADDNAME: []
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

        for kind, addrs in self.opcode_addrs.items():
            for name in self.opcode_names[kind]:
                addr = self.compute_op_addr(kind, name)
                addrs.append(addr)


    def get_op_addr(self, kind, name) -> int:
        op_tab = self.opcode_names[kind]
        op_idx = op_tab.index(name)
        return self.opcode_addrs[kind][op_idx]


    def compute_op_addr(self, kind, name) -> int:
        op_tab = self.opcode_names[kind]
        op_idx = op_tab.index(name)
        op_tab_addr = self.opcode_tab_addrs[kind] 
        offset = op_tab_addr + (op_idx * 2) 
        self.exe_fp.seek(offset - self.base_addr + self.exe_header_sz)

        # read word at (jump_table + offset)
        word = self.exe_fp.read(2)
        op_addr = op_tab_addr + int.from_bytes(word, "big")
        return op_addr


# =============================================================================
class SteemHelper:
    """_summary_
    """
    SECTION_DEBUG_ADDR = "Debug Addresses"
    KEY_ADDR = "Address"
    KEY_MODE = "Mode"
    KEY_BWR = "BWR"
    KEY_MASK_R = "MaskR"
    KEY_MASK_W = "MaskW"
    KEY_NAME = "Name"

    def __init__(self, alis_vm, steem_folder):
        """_summary_

        Args:
            alis_vm (_type_): _description_
            steem_folder (_type_): _description_
        """
        self.vm = alis_vm
        self.steem_folder = steem_folder
        self.ini_file = os.path.join(steem_folder, "steem.ini")
        self.config = configparser.ConfigParser()
        self.config.optionxform = str
        self.config.read(self.ini_file)
        self.bp_index = 0

        # get last breakpoint index
        if self.config.has_section(self.SECTION_DEBUG_ADDR):
            while self.config.has_option(self.SECTION_DEBUG_ADDR, self.KEY_ADDR + str(self.bp_index)):
                self.bp_index = self.bp_index + 1


    def commit(self):
        with open(self.ini_file, "w") as f:
            self.config.write(f)
        # steem is uber crappy when parsing ini files
        with open(self.ini_file, "r") as f:
            ini = f.read()
            ini = ini.replace(" = ", "=")
            with open(self.ini_file, "w") as f:
                f.writelines(ini)


    def add_all_bps(self, kind, mode=ESteemBreakMode.GLOBAL):
        names = self.vm.opcode_names[kind]
        self.add_bps(kind, names, mode)


    def add_bps(self, kind, names, mode=ESteemBreakMode.GLOBAL):
        """_summary_

        Args:
            kind (EAlisOpcodeKind): _description_
            names (String array): _description_
        """
        for name in names:
            self.add_bp(kind, name, mode)


    def add_bp(self, kind, name, mode=ESteemBreakMode.GLOBAL, bwr=1, mask_r=0, mask_w=0):
        """_summary_

        Args:
            kind (EAlisOpcodeKind): _description_
            name (String): _description_
            mode (ESteemBreakMode, optional): _description_. Defaults to GLOBAL.
            bwr (int, optional): _description_. Defaults to 0.
            mask_r (int, optional): _description_. Defaults to 0.
            mask_w (int, optional): _description_. Defaults to 0.
        """
        if self.config.has_section(self.SECTION_DEBUG_ADDR) == False:
            self.config.add_section(self.SECTION_DEBUG_ADDR)
        
        addr = self.vm.get_op_addr(kind, name)

        # check if bp exists for this addr
        bp_exists = False
        for (key, val) in self.config.items(self.SECTION_DEBUG_ADDR):
            if val == str(addr):
                bp_exists = True
                break

        if bp_exists == False:
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_ADDR + str(self.bp_index), str(addr))
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_MODE + str(self.bp_index), str(mode.value))
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_BWR + str(self.bp_index), str(bwr))
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_MASK_W + str(self.bp_index), str(mask_w))
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_MASK_R + str(self.bp_index), str(mask_r))
            self.config.set(self.SECTION_DEBUG_ADDR, self.KEY_NAME + str(self.bp_index), name)
            print(f"Created breakpoint for {kind}: {name} at address {str(hex(addr))}")
            self.bp_index = self.bp_index + 1
        
    
    def del_all_bps(self):
        if self.config.has_section(self.SECTION_DEBUG_ADDR):
            self.config.remove_section(self.SECTION_DEBUG_ADDR)


    def del_log(self):
        log = os.path.join(self.steem_folder, "steem.log")
        if os.path.exists(log):
            os.remove(log)



    # def handle_op(steem_lines,
    #             steem_line_idx,
    #             prefix_str,
    #             divider,
    #             op_idx,
    #             op_table,
    #             op_occurrences):
    #     #########################################
    #     regs_line = steem_lines[steem_line_idx + 1]
    #     addrs_line = steem_lines[steem_line_idx + 2]
    #     regs = regs_line.split("  ")
    #     addrs = addrs_line.split("  ")
    #     # d0 register contains the opcode in its lower byte
    #     d0_str = regs[0].split("=")[1]
    #     op_byte = int(int(d0_str[-2:], 16) / divider)
    #     # a3 register is the virtual PC address
    #     vpc = int(addrs[3].split("=")[1], 16) - 1
    #     op_name = op_table[op_byte]
    #     output_line = prefix_str + hex(vpc) + ": " + op_name

    #     # statistics
    #     if not op_name in op_occurrences.keys():
    #         op_occurrences[op_name] = 1
    #     else:
    #         count = op_occurrences[op_name]
    #         op_occurrences[op_name] = count + 1

    #     # next...
    #     #steem_line_idx += 2
    #     #op_idx += 1
    #     return output_line

    # def parse_log(self, alis_vm, output_file):
    #     steem_log = open(os.path.join(self.steem_folder, "steem.log"), "r")
    #     opcode_log = open(output_file, "w")
    #     steem_lines = steem_log.readlines()
    #     steem_lines_count = len(steem_lines)
    #     steem_line_idx = 0

    #     output_lines = []
    #     opcode_occurrences = {}
    #     opername_occurrences = {}
    #     storename_occurrences = {}
    #     addname_occurrences = {}

    #     opcode_idx = 0
    #     while steem_line_idx < steem_lines_count:
    #         steem_line = steem_lines[steem_line_idx]
    #         opcode_bp_addr = hex(alis_data.opcode_bp_addr)[2:]
    #         opername_bp_addr = hex(alis_data.opername_bp_addr)[2:]
    #         storename_bp_addr = hex(alis_data.storename_bp_addr)[2:]
    #         addname_bp_addr = hex(alis_data.addname_bp_addr)[2:]
    #         storename_bp_addr_2 = hex(alis_data.storename_bp_addr_2)[2:]
    #         addname_bp_addr_2 = hex(alis_data.addname_bp_addr_2)[2:]

    #         #########################################
    #         # FUN_READ_OPCODE
    #         if "$0" + str(opcode_bp_addr) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "", 1, opcode_idx,
    #                             alis_data.opcode_table, opcode_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         #########################################

    #         #########################################
    #         # FUN_READ_OPERNAME
    #         if "$0" + str(opername_bp_addr) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
    #                             alis_data.opername_table, opername_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         #########################################

    #         #########################################
    #         # FUN_READ_STORENAME
    #         if "$0" + str(storename_bp_addr) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
    #                             alis_data.storename_table, storename_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         if "$0" + str(storename_bp_addr_2) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
    #                             alis_data.storename_table, storename_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         #########################################

    #         #########################################
    #         # FUN_READ_ADDNAME
    #         if "$0" + str(addname_bp_addr) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
    #                             alis_data.addname_table, addname_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         if "$0" + str(addname_bp_addr_2) in steem_line:
    #             line = handle_op(steem_lines, steem_line_idx, "\t\t\tADDNAME", 2, opcode_idx,
    #                             alis_data.addname_table, addname_occurrences)
    #             output_lines.append(line)
    #             steem_line_idx += 2
    #             opcode_idx += 1
    #         #########################################

    #         # next line
    #         steem_line_idx += 1

    #     # print all called opcodes, in order
    #     print()
    #     print("HISTORY")
    #     print()
    #     for output_line in output_lines:
    #         print(output_line)
    #         opcode_log.write(output_line + "\n")

    #     # print opcode usage statistics
    #     print_stats("OPCODE", opcode_occurrences)
    #     print_stats("OPERNAME", opername_occurrences)
    #     print_stats("STORENAME", storename_occurrences)
    #     print_stats("ADDNAME", addname_occurrences)

    #     # close files
    #     steem_log.close()
    #     opcode_log.close()

# =============================================================================


vm = AlisVM(name="Ishar / Atari / uncracked / Daze", 
            exe_path="./data/ishar/atari/auto/start.prg",
            exe_md5="fa11cc8d6166d59edf143e4135ba058c",
            base_addr=0xaa9a,
            opcode_tab_addr=0x10b98,
            opername_tab_addr=0x10d5e,
            storename_tab_addr=0x10e08,
            addname_tab_addr=0x10e44)

sh = SteemHelper(vm, "./tools/Steem.SSE.4.0.2.Debug.Win32.DD")

sh.del_log()
sh.del_all_bps()
sh.commit()

sh.add_all_bps(EAlisOpcodeKind.STORENAME)
sh.commit()

