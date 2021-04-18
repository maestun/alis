
class AlisScript:

    HEADER_LEN = 24

    def __init__(self,
                 alisdata,
                 filename,
                 org):
        self.name = filename.split("/")[-1]
        f = open(filename, "rb")
        data = f.read()
        self.id = data[0]
        self.header = data[1]
        self.data = data
        self.org = org
        self.alis = alisdata
        f.close()

        print("Loaded script named " + self.name + " (" + str(len(self.data)) + " bytes) at " + hex(self.org))

    def disasm_operands(self,
                        address,
                        opcode):
        return ""

    def disasm_opcode(self,
                      address,
                      opcode):
        opname = self.alis.opcode_table[opcode]
        operands = self.disasm_operands(address + 1, opcode)
        line = hex(self.org + address) + ": " + opname + " " + operands
        return line

    def disasm(self):
        lines = []
        for pc in range(AlisScript.HEADER_LEN, len(self.data) - AlisScript.HEADER_LEN):
            line = self.disasm_opcode(pc, self.data[pc])
            lines.append(line)


class AlisData:
    def __init__(self,
                 name,
                 exe_name,
                 exe_md5,
                 opcode_bp_addr,
                 opername_bp_addr,
                 storename_bp_addr,
                 storename_bp_addr_2,
                 addname_bp_addr,
                 addname_bp_addr_2
                 ):
        self.name = name
        self.exe_name = exe_name
        self.exe_md5 = exe_md5
        self.opcode_bp_addr = opcode_bp_addr
        self.opername_bp_addr = opername_bp_addr
        self.storename_bp_addr = storename_bp_addr
        self.addname_bp_addr = addname_bp_addr
        self.storename_bp_addr_2 = storename_bp_addr_2
        self.addname_bp_addr_2 = addname_bp_addr_2
        self.opcode_table = ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", "cpointpix", "cchartmap", "cscsky", "czoom"]

        self.opername_table = ["oimmb", "oimmw", "oimmp", "olocb", "olocw", "olocp", "oloctp", "oloctc", "olocti", "odirb", "odirw", "odirp", "odirtp", "odirtc", "odirti", "omainb", "omainw", "omainp", "omaintp", "omaintc", "omainti", "ohimb", "ohimw", "ohimp", "ohimtp", "ohimtc", "ohimti", "opile", "oeval", "ofin", "cnul", "cnul", "opushacc", "oand", "oor", "oxor", "oeqv", "oegal", "odiff", "oinfeg", "osupeg", "oinf", "osup", "oadd", "osub", "omod", "odiv", "omul", "oneg", "oabs", "ornd", "osgn", "onot", "oinkey", "okeyon", "ojoy", "oprnd", "oscan", "oshiftkey", "ofree", "omodel", "ogetkey", "oleft", "oright", "omid", "olen", "oasc", "ostr", "osadd", "osegal", "osdiff", "osinfeg", "ossupeg", "osinf", "ossup", "ospushacc", "ospile", "oval", "oexistf", "ochr", "ochange", "ocountry", "omip", "ojoykey", "oconfig"]
        self.storename_table = ["cnul", "cnul", "cnul", "slocb", "slocw", "slocp", "sloctp", "sloctc", "slocti", "sdirb", "sdirw", "sdirp", "sdirtp", "sdirtc", "sdirti", "smainb", "smainw", "smainp", "smaintp", "smaintc", "smainti", "shimb", "shimw", "shimp", "shimtp", "shimtc", "shimti", "spile", "seval", "ofin"]
        self.addname_table = ["cnul", "cnul", "cnul", "alocb", "alocw", "alocp", "aloctp", "aloctc", "alocti", "adirb", "adirw", "adirp", "adirtp", "adirtc", "adirti", "amainb", "amainw", "amainp", "amaintp", "amaintc", "amainti", "ahimb", "ahimw", "ahimp", "ahimtp", "ahimtc", "ahimti", "spile", "aeval", "ofin"]

# ISHAR1_CRELITE = AlisData("Ishar 1 / Cr Elite",
#                           "START.PRG",
#                           "04e98965cb3187280d98f5ac9a8a0afd",
#                           opcode_bp_addr=,
#                           opername_bp_addr=,
#                           storename_bp_addr=,
#                           addname_bp_addr=)

ISHAR2_CRELITE = AlisData("Ishar 2 / Cr Elite",
                          "ISHAR2OK.PRG",
                          "87471ae02afacf5da303a99ce81ec1cd",
                          opcode_bp_addr=0x01310c,
                          opername_bp_addr=0x017572,
                          storename_bp_addr=0x013406, # in opcode_cstore, also at $01818a
                          storename_bp_addr_2=0x01818c,
                          addname_bp_addr=0x013438,
                          addname_bp_addr_2=0x01844c) # in fun_readexec_addname_swap, also at $01844a
