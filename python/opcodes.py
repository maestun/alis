
class AlisOpcodes():
    _names = ["cnul", "cesc1", "cesc2", "cesc3", "cbreakpt", "cjsr8", "cjsr16", "cjsr24", "cjmp8", "cjmp16", "cjmp24", 
              "cjsrabs", "cjmpabs", "cjsrind16", "cjsrind24", "cjmpind16", "cjmpind24", "cret", "cbz8", "cbz16", "cbz24", 
              "cbnz8", "cbnz16", "cbnz24", "cbeq8", "cbeq16", "cbeq24", "cbne8", "cbne16", "cbne24", "cstore", "ceval", 
              "cadd", "csub", "cmul", "cdiv", "cvprint", "csprinti", "csprinta", "clocate", "ctab", "cdim", "crandom", 
              "cloop8", "cloop16", "cloop24", "cswitch1", "cswitch2", "cstart8", "cstart16", "cstart24", "cleave", 
              "cprotect", "casleep", "cclock", "cnul", "cscmov", "cscset", "cclipping", "cswitching", "cwlive", "cunload", 
              "cwakeup", "csleep", "clive", "ckill", "cstop", "cstopret", "cexit", "cload", "cdefsc", "cscreen", "cput", 
              "cputnat", "cerase", "cerasen", "cset", "cmov", "copensc", "cclosesc", "cerasall", "cforme", "cdelforme", 
              "ctstmov", "ctstset", "cftstmov", "cftstset", "csuccent", "cpredent", "cnearent", "cneartyp", "cnearmat", 
              "cviewent", "cviewtyp", "cviewmat", "corient", "crstent", "csend", "cscanon", "cscanoff", "cinteron", "cinteroff", 
              "cscanclr", "callentity", "cpalette", "cdefcolor", "ctiming", "czap", "cexplode", "cding", "cnoise", "cinitab", 
              "cfopen", "cfclose", "cfcreat", "cfdel", "cfreadv", "cfwritev", "cfwritei", "cfreadb", "cfwriteb", "cplot", "cdraw", 
              "cbox", "cboxf", "cink", "cpset", "cpmove", "cpmode", "cpicture", "cxyscroll", "clinking", "cmouson", "cmousoff", 
              "cmouse", "cdefmouse", "csetmouse", "cdefvect", "csetvect", "cnul", "capproach", "cescape", "cvtstmov", "cvftstmov", 
              "cvmov", "cdefworld", "cworld", "cfindmat", "cfindtyp", "cmusic", "cdelmusic", "ccadence", "csetvolum", "cxinv", 
              "cxinvon", "cxinvoff", "clistent", "csound", "cmsound", "credon", "credoff", "cdelsound", "cwmov", "cwtstmov", 
              "cwftstmov", "ctstform", "cxput", "cxputat", "cmput", "cmputat", "cmxput", "cmxputat", "cmmusic", "cmforme", "csettime", 
              "cgettime", "cvinput", "csinput", "cnul", "cnul", "cnul", "crunfilm", "cvpicprint", "cspicprint", "cvputprint", 
              "csputprint", "cfont", "cpaper", "ctoblack", "cmovcolor", "ctopalet", "cnumput", "cscheart", "cscpos", "cscsize", 
              "cschoriz", "cscvertic", "cscreduce", "cscscale", "creducing", "cscmap", "cscdump", "cfindcla", "cnearcla", 
              "cviewcla", "cinstru", "cminstru", "cordspr", "calign", "cbackstar", "cstarring", "cengine", "cautobase", 
              "cquality", "chsprite", "cselpalet", "clinepalet", "cautomode", "cautofile", "ccancel", "ccancall", "ccancen", 
              "cblast", "cscback", "cscrolpage", "cmatent", "cshrink", "cdefmap", "csetmap", "cputmap", "csavepal", "csczoom", 
              "ctexmap", "calloctab", "cfreetab", "cscantab", "cneartab", "cscsun", "cdarkpal", "cscdark", "caset", "camov", 
              "cscaset", "cscamov", "cscfollow", "cscview", "cfilm", "cwalkmap", "catstmap", "cavtstmov", "cavmov", "caim", 
              "cpointpix", "cchartmap", "cscsky", "czoom"]

    def __init__(self) -> None:
        pass

