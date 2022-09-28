# Ishar II / Atari / cracked by Elite
# ./data/ishar2/atari/auto/ISHAR2OK.PRG
# 87471ae02afacf5da303a99ce81ec1cd
# 2022-09-21 17:25:10

from enum import Enum

def set_bit(value, bit):
    return value | (1<<bit)

def clear_bit(value, bit):
    return value & ~(1<<bit)

# =============================================================================
class EAlisOpcodeKind(str, Enum):
    OPCODE = "opcode"
    OPERAND = "operand"
    STORENAME = "storename"
    ADDNAME = "addname"


# =============================================================================
class AlisOpcode():
    def __init__(self, code, fptr, desc, addr):
        self.code = code
        self.fptr = fptr
        self.name = fptr.__name__
        self.desc = desc
        self.addr = addr


# =============================================================================
# OPCODE
# =============================================================================
# 0x1311c: cnul (0x0)
def cnul(vm):
	print("cnul is MISSING...")

# 0x1311e: cesc1 (0x1)
def cesc1(vm):
	print("cesc1 is MISSING...")

# 0x13120: cesc2 (0x2)
def cesc2(vm):
	print("cesc2 is MISSING...")

# 0x13122: cesc3 (0x3)
def cesc3(vm):
	print("cesc3 is MISSING...")

# 0x12fa6: cbreakpt (0x4)
def cbreakpt(vm):
	print("cbreakpt is N/I...")

# 0x13126: cjsr8 (0x5)
def cjsr8(vm):
	# print("cjsr8 is TO TEST...")
	offset = vm.script.cread(1)
	cjsr(offset)

# 0x13134: cjsr16 (0x6)
def cjsr16(vm):
	# print("cjsr16 is TO TEST...")
	offset = vm.script.cread(2)
	cjsr(offset)

# 0x13144: cjsr24 (0x7)
def cjsr24(vm):
	# print("cjsr24 is TO TEST...")
	offset = vm.script.cread(3)
	cjsr(vm, offset)

# 0x1315a: cjmp8 (0x8)
def cjmp8(vm):
	print("cjmp8 is MISSING...")

# 0x13162: cjmp16 (0x9)
def cjmp16(vm):
	print("cjmp16 is MISSING...")

# 0x1316c: cjmp24 (0xa)
def cjmp24(vm):
	cjmp(vm, vm.script.cread(3))

# 0x1323e: cjsrabs (0xb)
def cjsrabs(vm):
	print("cjsrabs is MISSING...")

# 0x13240: cjmpabs (0xc)
def cjmpabs(vm):
	print("cjmpabs is MISSING...")

# 0x13242: cjsrind16 (0xd)
def cjsrind16(vm):
	print("cjsrind16 is MISSING...")

# 0x13244: cjsrind24 (0xe)
def cjsrind24(vm):
	print("cjsrind24 is MISSING...")

# 0x13246: cjmpind16 (0xf)
def cjmpind16(vm):
	print("cjmpind16 is MISSING...")

# 0x13248: cjmpind24 (0x10)
def cjmpind24(vm):
	print("cjmpind24 is MISSING...")

# 0x1324a: cret (0x11)
def cret(vm):
	# print("cret is TO TEST...")
	# return from subroutine (cjsr)
    # retrieve return address **OFFSET** from virtual stack
	pc = vm.acc.pop()
	vm.script.jump(pc, False)

# 0x132e2: cbz8 (0x12)
def cbz8(vm):
	print("cbz8 is MISSING...")

# 0x132f4: cbz16 (0x13)
def cbz16(vm):
	print("cbz16 is MISSING...")

# 0x13308: cbz24 (0x14)
def cbz24(vm):
	cbz(vm, vm.script.cread(3))

# 0x13322: cbnz8 (0x15)
def cbnz8(vm):
	print("cbnz8 is MISSING...")

# 0x13334: cbnz16 (0x16)
def cbnz16(vm):
	print("cbnz16 is MISSING...")

# 0x13348: cbnz24 (0x17)
def cbnz24(vm):
	print("cbnz24 is MISSING...")

# 0x13362: cbeq8 (0x18)
def cbeq8(vm):
	print("cbeq8 is MISSING...")

# 0x13374: cbeq16 (0x19)
def cbeq16(vm):
	print("cbeq16 is MISSING...")

# 0x13388: cbeq24 (0x1a)
def cbeq24(vm):
	print("cbeq24 is MISSING...")

# 0x133a2: cbne8 (0x1b)
def cbne8(vm):
	print("cbne8 is MISSING...")

# 0x133b4: cbne16 (0x1c)
def cbne16(vm):
	print("cbne16 is MISSING...")

# 0x133c8: cbne24 (0x1d)
def cbne24(vm):
	print("cbne24 is MISSING...")

# 0x133e2: cstore (0x1e)
def cstore(vm):
	# print("cstore is TO TEST...")
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	cstore_continue(vm)

# 0x1340e: ceval (0x1f)
def ceval(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)

# 0x13414: cadd (0x20)
def cadd(vm):
	print("cadd is MISSING...")

# 0x13440: csub (0x21)
def csub(vm):
	print("csub is MISSING...")

# 0x1344a: cmul (0x22)
def cmul(vm):
	print("cmul is MISSING...")

# 0x1344c: cdiv (0x23)
def cdiv(vm):
	print("cdiv is MISSING...")

# 0x135de: cvprint (0x24)
def cvprint(vm):
	print("cvprint is MISSING...")

# 0x135ee: csprinti (0x25)
def csprinti(vm):
	print("csprinti is MISSING...")

# 0x13604: csprinta (0x26)
def csprinta(vm):
	print("csprinta is MISSING...")

# 0x1363a: clocate (0x27)
def clocate(vm):
	print("clocate is MISSING...")

# 0x1365a: ctab (0x28)
def ctab(vm):
	print("ctab is MISSING...")

# 0x15fea: cdim (0x29)
def cdim(vm):
	# print("cdim is TO TEST...")
	# read word param
	offset = vm.script.cread(2)
	counter = vm.script.cread(1)
	byte2 = vm.script.cread(1)

	offset -= 1
	vm.ram.write(offset, counter, 1)
	offset -= 1
	vm.ram.write(offset, byte2, 1)

	# loop w/ counter, read words, store backwards
	while counter > 0:
		w = vm.script.cread(2)
		offset -= 2
		vm.ram.write(offset, w, 2)
		counter -= 1


# 0x13672: crandom (0x2a)
def crandom(vm):
	vm.readexec(EAlisOpcodeKind.OPERAND)
	if vm.vars.d7 == 0:
		vm.vars.d7 = vm.sys.random()
		if vm.vars.b_automode != 0:
			vm.vars.d7 = 100
	vm.vars.w_crnd = vm.vars.d7

# 0x13272: cloop8 (0x2b)
def cloop8(vm):
	print("cloop8 is MISSING...")

# 0x13294: cloop16 (0x2c)
def cloop16(vm):
	print("cloop16 is MISSING...")

# 0x132b8: cloop24 (0x2d)
def cloop24(vm):
	print("cloop24 is MISSING...")

# 0x16016: cswitch1 (0x2e)
def cswitch1(vm):
	print("cswitch1 is MISSING...")

# 0x1604a: cswitch2 (0x2f)
def cswitch2(vm):
	print("cswitch2 is MISSING...")

# 0x1317c: cstart8 (0x30)
def cstart8(vm):
	print("cstart8 is MISSING...")

# 0x13186: cstart16 (0x31)
def cstart16(vm):
	print("cstart16 is MISSING...")

# 0x131f6: cstart24 (0x32)
def cstart24(vm):
	print("cstart24 is MISSING...")

# 0x13206: cleave (0x33)
def cleave(vm):
	print("cleave is MISSING...")

# 0x17418: cprotect (0x34)
def cprotect(vm):
	print("cprotect is MISSING...")

# 0x1607c: casleep (0x35)
def casleep(vm):
	print("casleep is MISSING...")

# 0x1311c: cclock (0x36)
def cclock(vm):
	print("cclock is MISSING...")

# 0x1311c: cnul (0x37)
def cnul(vm):
	print("cnul is MISSING...")

# 0x13aa6: cscmov (0x38)
def cscmov(vm):
	print("cscmov is MISSING...")

# 0x13ad4: cscset (0x39)
def cscset(vm):
	print("cscset is MISSING...")

# 0x13a9e: cclipping (0x3a)
def cclipping(vm):
	print("cclipping is MISSING...")

# 0x13a8a: cswitching (0x3b)
def cswitching(vm):
	if vm.vars.b_tvmode == 0:
		vm.vars.w_fswitch = 1

# 0x138bc: cwlive (0x3c)
def cwlive(vm):
	print("cwlive is MISSING...")

# 0x139c0: cunload (0x3d)
def cunload(vm):
	print("cunload is MISSING...")

# 0x160a6: cwakeup (0x3e)
def cwakeup(vm):
	print("cwakeup is MISSING...")

# 0x16096: csleep (0x3f)
def csleep(vm):
	print("csleep is MISSING...")

# 0x138ce: clive (0x40)
def clive(vm):
	print("clive is STUBBED...")
	vm.vars.w_cx = 0
	vm.vars.w_cy = 0
	vm.vars.w_cz = 0

	d2 = vm.script.cread(2)
	# TODO: debprotf
	# TODO: liveprog
	tmp = vm.vars.sd7
	vm.vars.sd7 = vm.vars.oldsd7
	vm.vars.oldsd7 = tmp
	vm.readexec(EAlisOpcodeKind.OPERAND)


# 0x13962: ckill (0x41)
def ckill(vm):
	print("ckill is MISSING...")

# 0x13972: cstop (0x42)
def cstop(vm):
	print("cstop is MISSING...")
	# in real program, adds 4 to real stack pointer
	vm.script.stop = True

# 0x13976: cstopret (0x43)
def cstopret(vm):
	print("cstopret is MISSING...")

# 0x13988: cexit (0x44)
def cexit(vm):
	print("cexit is MISSING...")

# 0x13998: cload (0x45)
def cload(vm):
	print("cload is TO TEST...")
	id = vm.script.cread(2)
	if id == 0:
		tmp = vm.vars.sd7
		vm.vars.sd7 = vm.vars.oldsd7
		vm.vars.oldsd7 = tmp
		vm.readexec(EAlisOpcodeKind.OPERAND)
		# TODO: handle new main
	else:
		# skip file name
		name = bytearray(vm.script.cread(0))[:-1].decode()
		print("CLOAD: " + name)

# 0x139ca: cdefsc (0x46)
def cdefsc(vm):
	print("cdefsc is STUBBED...")
	vm.script.pc += 35

# 0x13b12: cscreen (0x47)
def cscreen(vm):
	d0 = vm.script.cread(2)
	if d0 != vm.ctx._0x22_cscreen:
		vm.ctx._0x22_cscreen = d0

# 0x13e58: cput (0x48)
def cput(vm):
	print("cput is MISSING...")

# 0x13e82: cputnat (0x49)
def cputnat(vm):
	print("cputnat is MISSING...")

# 0x1455a: cerase (0x4a)
def cerase(vm):
	print("cerase is MISSING...")

# 0x1452a: cerasen (0x4b)
def cerasen(vm):
	print("cerasen is MISSING...")

# 0x14652: cset (0x4c)
def cset(vm):
	print("cset is MISSING...")

# 0x1466c: cmov (0x4d)
def cmov(vm):
	print("cmov is MISSING...")

# 0x13d4c: copensc (0x4e)
def copensc(vm):
	d0 = vm.script.cread(2)
	a0 = vm.vars.addr_basemain
	b = vm.ram.read(a0 + d0, 1)
	b = clear_bit(b, 6)
	b = set_bit(b, 7)
	vm.ram.write(a0 + d0, b, 1)
	scbreak(vm)
	scadd(vm)

# 0x13d6e: cclosesc (0x4f)
def cclosesc(vm):
	print("cclosesc is MISSING...")

# 0x1458e: cerasall (0x50)
def cerasall(vm):
	print("cerasall is MISSING...")

# 0x1534c: cforme (0x51)
def cforme(vm):
	print("cforme is MISSING...")

# 0x15362: cdelforme (0x52)
def cdelforme(vm):
	print("cdelforme is MISSING...")

# 0x154ce: ctstmov (0x53)
def ctstmov(vm):
	print("ctstmov is MISSING...")

# 0x155b4: ctstset (0x54)
def ctstset(vm):
	print("ctstset is MISSING...")

# 0x155ec: cftstmov (0x55)
def cftstmov(vm):
	print("cftstmov is MISSING...")

# 0x1563e: cftstset (0x56)
def cftstset(vm):
	print("cftstset is MISSING...")

# 0x15684: csuccent (0x57)
def csuccent(vm):
	print("csuccent is MISSING...")

# 0x1569a: cpredent (0x58)
def cpredent(vm):
	print("cpredent is MISSING...")

# 0x15e6c: cnearent (0x59)
def cnearent(vm):
	print("cnearent is MISSING...")

# 0x15cf6: cneartyp (0x5a)
def cneartyp(vm):
	print("cneartyp is MISSING...")

# 0x15c40: cnearmat (0x5b)
def cnearmat(vm):
	print("cnearmat is MISSING...")

# 0x15f36: cviewent (0x5c)
def cviewent(vm):
	print("cviewent is MISSING...")

# 0x15eea: cviewtyp (0x5d)
def cviewtyp(vm):
	print("cviewtyp is MISSING...")

# 0x15ebe: cviewmat (0x5e)
def cviewmat(vm):
	print("cviewmat is MISSING...")

# 0x146f2: corient (0x5f)
def corient(vm):
	print("corient is MISSING...")

# 0x156be: crstent (0x60)
def crstent(vm):
	print("crstent is MISSING...")

# 0x160c2: csend (0x61)
def csend(vm):
	print("csend is MISSING...")

# 0x161a8: cscanon (0x62)
def cscanon(vm):
	# print("cscanon is TO TEST...")
	vm.ctx._0x24_scan_off_bit_0 = False

# 0x1619c: cscanoff (0x63)
def cscanoff(vm):
	print("cscanoff is TO TEST...")
	vm.ctx._0x24_scan_off_bit_0 = True
	cscanclr(vm)

# 0x161b8: cinteron (0x64)
def cinteron(vm):
	print("cinteron is TO TEST...")
	vm.ctx._0x24_inter_off_bit_1 = False

# 0x161b0: cinteroff (0x65)
def cinteroff(vm):
	print("cinteroff is TO TEST...")
	vm.ctx._0x24_inter_off_bit_1 = True

# 0x1618e: cscanclr (0x66)
def cscanclr(vm):
	print("cscanclr is TO TEST...")
	vm.ctx._0x1e_scan_clr = vm.ctx._0x1c_scan_clr
	vm.ctx._0x24_scan_clr_bit_7 = False


# 0x15fde: callentity (0x67)
def callentity(vm):
	print("callentity is MISSING...")

# 0x162d0: cpalette (0x68)
def cpalette(vm):
	print("cpalette is MISSING...")

# 0x16404: cdefcolor (0x69)
def cdefcolor(vm):
	print("cdefcolor is MISSING...")

# 0x161c0: ctiming (0x6a)
def ctiming(vm):
	# print("ctiming is TO TEST...")
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.b_timing = vm.vars.d7 & 0xff

# 0x16876: czap (0x6b)
def czap(vm):
	print("czap is MISSING...")

# 0x168cc: cexplode (0x6c)
def cexplode(vm):
	print("cexplode is MISSING...")

# 0x169b6: cding (0x6d)
def cding(vm):
	print("cding is MISSING...")

# 0x16938: cnoise (0x6e)
def cnoise(vm):
	print("cnoise is MISSING...")

# 0x161cc: cinitab (0x6f)
def cinitab(vm):
	print("cinitab is MISSING...")

# 0x16f3c: cfopen (0x70)
def cfopen(vm):
	print("cfopen is MISSING...")

# 0x16f6c: cfclose (0x71)
def cfclose(vm):
	print("cfclose is MISSING...")

# 0x16f70: cfcreat (0x72)
def cfcreat(vm):
	print("cfcreat is MISSING...")

# 0x16f88: cfdel (0x73)
def cfdel(vm):
	print("cfdel is MISSING...")

# 0x16f94: cfreadv (0x74)
def cfreadv(vm):
	print("cfreadv is MISSING...")

# 0x16faa: cfwritev (0x75)
def cfwritev(vm):
	print("cfwritev is MISSING...")

# 0x16fc2: cfwritei (0x76)
def cfwritei(vm):
	print("cfwritei is MISSING...")

# 0x16fd4: cfreadb (0x77)
def cfreadb(vm):
	print("cfreadb is MISSING...")

# 0x17000: cfwriteb (0x78)
def cfwriteb(vm):
	print("cfwriteb is MISSING...")

# 0x1702c: cplot (0x79)
def cplot(vm):
	print("cplot is MISSING...")

# 0x1704c: cdraw (0x7a)
def cdraw(vm):
	print("cdraw is MISSING...")

# 0x1707c: cbox (0x7b)
def cbox(vm):
	print("cbox is MISSING...")

# 0x170d0: cboxf (0x7c)
def cboxf(vm):
	print("cboxf is MISSING...")

# 0x17100: cink (0x7d)
def cink(vm):
	print("cink is MISSING...")

# 0x17128: cpset (0x7e)
def cpset(vm):
	print("cpset is MISSING...")

# 0x1713e: cpmove (0x7f)
def cpmove(vm):
	print("cpmove is MISSING...")

# 0x17154: cpmode (0x80)
def cpmode(vm):
	print("cpmode is MISSING...")

# 0x17168: cpicture (0x81)
def cpicture(vm):
	print("cpicture is MISSING...")

# 0x1740e: cxyscroll (0x82)
def cxyscroll(vm):
	print("cxyscroll is MISSING...")

# 0x14214: clinking (0x83)
def clinking(vm):
	print("clinking is MISSING...")

# 0x17302: cmouson (0x84)
def cmouson(vm):
	print("cmouson is MISSING...")

# 0x17328: cmousoff (0x85)
def cmousoff(vm):
	print("cmousoff is MISSING...")

# 0x1735a: cmouse (0x86)
def cmouse(vm):
	print("cmouse is MISSING...")

# 0x17390: cdefmouse (0x87)
def cdefmouse(vm):
	print("cdefmouse is MISSING...")

# 0x173be: csetmouse (0x88)
def csetmouse(vm):
	print("csetmouse is MISSING...")

# 0x173d2: cdefvect (0x89)
def cdefvect(vm):
	print("cdefvect is MISSING...")

# 0x173e4: csetvect (0x8a)
def csetvect(vm):
	print("csetvect is MISSING...")

# 0x1311c: cnul (0x8b)
def cnul(vm):
	print("cnul is MISSING...")

# 0x174e2: capproach (0x8c)
def capproach(vm):
	print("capproach is MISSING...")

# 0x1752c: cescape (0x8d)
def cescape(vm):
	print("cescape is MISSING...")

# 0x15512: cvtstmov (0x8e)
def cvtstmov(vm):
	print("cvtstmov is MISSING...")

# 0x1555c: cvftstmov (0x8f)
def cvftstmov(vm):
	print("cvftstmov is MISSING...")

# 0x14686: cvmov (0x90)
def cvmov(vm):
	print("cvmov is MISSING...")

# 0x173f0: cdefworld (0x91)
def cdefworld(vm):
	print("cdefworld is MISSING...")

# 0x17404: cworld (0x92)
def cworld(vm):
	print("cworld is MISSING...")

# 0x15aae: cfindmat (0x93)
def cfindmat(vm):
	print("cfindmat is MISSING...")

# 0x15b04: cfindtyp (0x94)
def cfindtyp(vm):
	print("cfindtyp is MISSING...")

# 0x16d8c: cmusic (0x95)
def cmusic(vm):
	print("cmusic is MISSING...")

# 0x16e76: cdelmusic (0x96)
def cdelmusic(vm):
	print("cdelmusic is MISSING...")

# 0x16ba8: ccadence (0x97)
def ccadence(vm):
	print("ccadence is MISSING...")

# 0x16b90: csetvolum (0x98)
def csetvolum(vm):
	print("csetvolum is MISSING...")

# 0x13e50: cxinv (0x99)
def cxinv(vm):
	print("cxinv is MISSING...")

# 0x13e42: cxinvon (0x9a)
def cxinvon(vm):
	print("cxinvon is MISSING...")

# 0x13e4a: cxinvoff (0x9b)
def cxinvoff(vm):
	print("cxinvoff is MISSING...")

# 0x15bb2: clistent (0x9c)
def clistent(vm):
	print("clistent is MISSING...")

# 0x16e94: csound (0x9d)
def csound(vm):
	print("csound is MISSING...")

# 0x16f28: cmsound (0x9e)
def cmsound(vm):
	print("cmsound is MISSING...")

# 0x14230: credon (0x9f)
def credon(vm):
	print("credon is MISSING...")

# 0x14236: credoff (0xa0)
def credoff(vm):
	print("credoff is MISSING...")

# 0x16f34: cdelsound (0xa1)
def cdelsound(vm):
	print("cdelsound is MISSING...")

# 0x146a6: cwmov (0xa2)
def cwmov(vm):
	print("cwmov is MISSING...")

# 0x15404: cwtstmov (0xa3)
def cwtstmov(vm):
	print("cwtstmov is MISSING...")

# 0x15442: cwftstmov (0xa4)
def cwftstmov(vm):
	print("cwftstmov is MISSING...")

# 0x1548c: ctstform (0xa5)
def ctstform(vm):
	print("ctstform is MISSING...")

# 0x13ec2: cxput (0xa6)
def cxput(vm):
	print("cxput is MISSING...")

# 0x13edc: cxputat (0xa7)
def cxputat(vm):
	print("cxputat is MISSING...")

# 0x13ef4: cmput (0xa8)
def cmput(vm):
	print("cmput is MISSING...")

# 0x13f08: cmputat (0xa9)
def cmputat(vm):
	print("cmputat is MISSING...")

# 0x13f36: cmxput (0xaa)
def cmxput(vm):
	print("cmxput is MISSING...")

# 0x13f1c: cmxputat (0xab)
def cmxputat(vm):
	print("cmxputat is MISSING...")

# 0x16d80: cmmusic (0xac)
def cmmusic(vm):
	print("cmmusic is MISSING...")

# 0x1536a: cmforme (0xad)
def cmforme(vm):
	print("cmforme is MISSING...")

# 0x1387e: csettime (0xae)
def csettime(vm):
	print("csettime is MISSING...")

# 0x138a0: cgettime (0xaf)
def cgettime(vm):
	print("cgettime is MISSING...")

# 0x134bc: cvinput (0xb0)
def cvinput(vm):
	print("cvinput is MISSING...")

# 0x134ac: csinput (0xb1)
def csinput(vm):
	print("csinput is MISSING...")

# 0x1311c: cnul (0xb2)
def cnul(vm):
	print("cnul is MISSING...")

# 0x1311c: cnul (0xb3)
def cnul(vm):
	print("cnul is MISSING...")

# 0x1311c: cnul (0xb4)
def cnul(vm):
	print("cnul is MISSING...")

# 0x1311c: crunfilm (0xb5)
def crunfilm(vm):
	print("crunfilm is MISSING...")

# 0x135a8: cvpicprint (0xb6)
def cvpicprint(vm):
	print("cvpicprint is MISSING...")

# 0x13596: cspicprint (0xb7)
def cspicprint(vm):
	print("cspicprint is MISSING...")

# 0x135cc: cvputprint (0xb8)
def cvputprint(vm):
	print("cvputprint is MISSING...")

# 0x135ba: csputprint (0xb9)
def csputprint(vm):
	print("csputprint is MISSING...")

# 0x1344e: cfont (0xba)
def cfont(vm):
	print("cfont is MISSING...")

# 0x17114: cpaper (0xbb)
def cpaper(vm):
	print("cpaper is MISSING...")

# 0x1674a: ctoblack (0xbc)
def ctoblack(vm):
	print("ctoblack is MISSING...")

# 0x1644e: cmovcolor (0xbd)
def cmovcolor(vm):
	print("cmovcolor is MISSING...")

# 0x16542: ctopalet (0xbe)
def ctopalet(vm):
	print("ctopalet is MISSING...")

# 0x13482: cnumput (0xbf)
def cnumput(vm):
	print("cnumput is MISSING...")

# 0x13b26: cscheart (0xc0)
def cscheart(vm):
	print("cscheart is MISSING...")

# 0x13b72: cscpos (0xc1)
def cscpos(vm):
	print("cscpos is MISSING...")

# 0x13b4c: cscsize (0xc2)
def cscsize(vm):
	print("cscsize is MISSING...")

# 0x13b98: cschoriz (0xc3)
def cschoriz(vm):
	print("cschoriz is MISSING...")

# 0x13bd2: cscvertic (0xc4)
def cscvertic(vm):
	print("cscvertic is MISSING...")

# 0x13c62: cscreduce (0xc5)
def cscreduce(vm):
	print("cscreduce is MISSING...")

# 0x13c9c: cscscale (0xc6)
def cscscale(vm):
	print("cscscale is MISSING...")

# 0x1421e: creducing (0xc7)
def creducing(vm):
	print("creducing is MISSING...")

# 0x13cba: cscmap (0xc8)
def cscmap(vm):
	print("cscmap is MISSING...")

# 0x13cbc: cscdump (0xc9)
def cscdump(vm):
	print("cscdump is MISSING...")

# 0x15b60: cfindcla (0xca)
def cfindcla(vm):
	print("cfindcla is MISSING...")

# 0x15db6: cnearcla (0xcb)
def cnearcla(vm):
	print("cnearcla is MISSING...")

# 0x15f10: cviewcla (0xcc)
def cviewcla(vm):
	print("cviewcla is MISSING...")

# 0x16c10: cinstru (0xcd)
def cinstru(vm):
	print("cinstru is MISSING...")

# 0x16c04: cminstru (0xce)
def cminstru(vm):
	print("cminstru is MISSING...")

# 0x14282: cordspr (0xcf)
def cordspr(vm):
	print("cordspr is MISSING...")

# 0x1423e: calign (0xd0)
def calign(vm):
	print("calign is MISSING...")

# 0x167b0: cbackstar (0xd1)
def cbackstar(vm):
	print("cbackstar is MISSING...")

# 0x16814: cstarring (0xd2)
def cstarring(vm):
	print("cstarring is MISSING...")

# 0x169a4: cengine (0xd3)
def cengine(vm):
	print("cengine is MISSING...")

# 0x179dc: cautobase (0xd4)
def cautobase(vm):
	print("cautobase is MISSING...")

# 0x16e88: cquality (0xd5)
def cquality(vm):
	print("cquality is MISSING...")

# 0x1428c: chsprite (0xd6)
def chsprite(vm):
	print("chsprite is MISSING...")

# 0x16288: cselpalet (0xd7)
def cselpalet(vm):
	print("cselpalet is MISSING...")

# 0x161e6: clinepalet (0xd8)
def clinepalet(vm):
	print("clinepalet is MISSING...")

# 0x179d0: cautomode (0xd9)
def cautomode(vm):
	print("cautomode is MISSING...")

# 0x179d6: cautofile (0xda)
def cautofile(vm):
	print("cautofile is MISSING...")

# 0x14552: ccancel (0xdb)
def ccancel(vm):
	print("ccancel is MISSING...")

# 0x14586: ccancall (0xdc)
def ccancall(vm):
	print("ccancall is MISSING...")

# 0x14522: ccancen (0xdd)
def ccancen(vm):
	print("ccancen is MISSING...")

# 0x1395a: cblast (0xde)
def cblast(vm):
	print("cblast is MISSING...")

# 0x13d04: cscback (0xdf)
def cscback(vm):
	print("cscback is MISSING...")

# 0x13cc0: cscrolpage (0xe0)
def cscrolpage(vm):
	print("cscrolpage is MISSING...")

# 0x156cc: cmatent (0xe1)
def cmatent(vm):
	print("cmatent is MISSING...")

# 0x189f8: cshrink (0xe2)
def cshrink(vm):
	print("cshrink is MISSING...")

# 0x13cbe: cdefmap (0xe3)
def cdefmap(vm):
	print("cdefmap is MISSING...")

# 0x13cbe: csetmap (0xe4)
def csetmap(vm):
	print("csetmap is MISSING...")

# 0x13cbe: cputmap (0xe5)
def cputmap(vm):
	print("cputmap is MISSING...")

# 0x16350: csavepal (0xe6)
def csavepal(vm):
	print("csavepal is MISSING...")

# 0x173c6: csczoom (0xe7)
def csczoom(vm):
	print("csczoom is MISSING...")

# 0x173cc: ctexmap (0xe8)
def ctexmap(vm):
	print("ctexmap is MISSING...")

# 0x173d4: calloctab (0xe9)
def calloctab(vm):
	print("calloctab is MISSING...")

# 0x173e2: cfreetab (0xea)
def cfreetab(vm):
	print("cfreetab is MISSING...")

# 0x173f0: cscantab (0xeb)
def cscantab(vm):
	print("cscantab is MISSING...")

# 0x173fc: cneartab (0xec)
def cneartab(vm):
	print("cneartab is MISSING...")

# 0x17436: cscsun (0xed)
def cscsun(vm):
	print("cscsun is MISSING...")

# 0x17414: cdarkpal (0xee)
def cdarkpal(vm):
	print("cdarkpal is MISSING...")

# 0x17426: cscdark (0xef)
def cscdark(vm):
	print("cscdark is MISSING...")

# 0x17452: caset (0xf0)
def caset(vm):
	print("caset is MISSING...")

# 0x1745e: camov (0xf1)
def camov(vm):
	print("camov is MISSING...")

# 0x17468: cscaset (0xf2)
def cscaset(vm):
	print("cscaset is MISSING...")

# 0x1749c: cscamov (0xf3)
def cscamov(vm):
	print("cscamov is MISSING...")

# 0x1747e: cscfollow (0xf4)
def cscfollow(vm):
	print("cscfollow is MISSING...")

# 0x1748e: cscview (0xf5)
def cscview(vm):
	print("cscview is MISSING...")

# 0x174b6: cfilm (0xf6)
def cfilm(vm):
	print("cfilm is MISSING...")

# 0x174ca: cwalkmap (0xf7)
def cwalkmap(vm):
	print("cwalkmap is MISSING...")

# 0x174dc: catstmap (0xf8)
def catstmap(vm):
	print("catstmap is MISSING...")

# 0x17530: cavtstmov (0xf9)
def cavtstmov(vm):
	print("cavtstmov is MISSING...")

# 0x174fa: cavmov (0xfa)
def cavmov(vm):
	print("cavmov is MISSING...")

# 0x17516: caim (0xfb)
def caim(vm):
	print("caim is MISSING...")

# 0x17556: cpointpix (0xfc)
def cpointpix(vm):
	print("cpointpix is MISSING...")

# 0x17578: cchartmap (0xfd)
def cchartmap(vm):
	print("cchartmap is MISSING...")

# 0x17598: cscsky (0xfe)
def cscsky(vm):
	print("cscsky is MISSING...")

# 0x17616: czoom (0xff)
def czoom(vm):
	print("czoom is MISSING...")


# =============================================================================
# OPERAND
# =============================================================================
# 0x17594: oimmb (0x0)
def oimmb(vm):
	# print("oimmb is TO TEST...")
	# reads a byte, extends into r7
	vm.vars.d7 = vm.script.cread(1, True)

# 0x1759a: oimmw (0x1)
def oimmw(vm):
	# print("oimmw is TO TEST...")
	vm.vars.d7 = vm.script.cread(2)

# 0x175a2: oimmp (0x2)
def oimmp(vm):
	vm.vars.sd7 = vm.script.cread(0)

# 0x175b0: olocb (0x3)
def olocb(vm):
	# print("olocb is TO TEST...")
	offset = vm.script.cread(2)
	# TODO: it's offset + basemain
	vm.vars.d7 = vm.ram.read(offset, 1, True)

# 0x175be: olocw (0x4)
def olocw(vm):
	print("olocw is TO TEST...")
	offset = vm.script.cread(2)
	# TODO: it's offset + basemain
	vm.vars.d7 = vm.ram.read(offset, 2)

# 0x175ca: olocp (0x5)
def olocp(vm):
	offset = vm.script.cread(2)
	# TODO: it's basemain + offset
	vm.vars.sd7 = vm.ram.readp(offset)

# 0x17604: oloctp (0x6)
def oloctp(vm):
	print("oloctp is MISSING...")

# 0x175e2: oloctc (0x7)
def oloctc(vm):
	print("oloctc is MISSING...")

# 0x175f4: olocti (0x8)
def olocti(vm):
	print("olocti is MISSING...")

# 0x17620: odirb (0x9)
def odirb(vm):
	print("odirb is MISSING...")

# 0x1762c: odirw (0xa)
def odirw(vm):
	print("odirw is MISSING...")

# 0x17636: odirp (0xb)
def odirp(vm):
	print("odirp is MISSING...")

# 0x1766a: odirtp (0xc)
def odirtp(vm):
	print("odirtp is MISSING...")

# 0x1764c: odirtc (0xd)
def odirtc(vm):
	print("odirtc is MISSING...")

# 0x1765c: odirti (0xe)
def odirti(vm):
	print("odirti is MISSING...")

# 0x17684: omainb (0xf)
def omainb(vm):
	print("omainb is MISSING...")

# 0x17698: omainw (0x10)
def omainw(vm):
	print("omainw is MISSING...")

# 0x176aa: omainp (0x11)
def omainp(vm):
	print("omainp is MISSING...")

# 0x176fe: omaintp (0x12)
def omaintp(vm):
	print("omaintp is MISSING...")

# 0x176c8: omaintc (0x13)
def omaintc(vm):
	print("omaintc is MISSING...")

# 0x176e4: omainti (0x14)
def omainti(vm):
	print("omainti is MISSING...")

# 0x17724: ohimb (0x15)
def ohimb(vm):
	print("ohimb is MISSING...")

# 0x17746: ohimw (0x16)
def ohimw(vm):
	print("ohimw is MISSING...")

# 0x17766: ohimp (0x17)
def ohimp(vm):
	print("ohimp is MISSING...")

# 0x177e4: ohimtp (0x18)
def ohimtp(vm):
	print("ohimtp is MISSING...")

# 0x17792: ohimtc (0x19)
def ohimtc(vm):
	print("ohimtc is MISSING...")

# 0x177bc: ohimti (0x1a)
def ohimti(vm):
	print("ohimti is MISSING...")

# 0x17f00: opile (0x1b)
def opile(vm):
	vm.vars.d7 = vm.vars.d6
	vm.vars.d6 = vm.acc.pop()

# 0x17f06: oeval (0x1c)
def oeval(vm):
	# print("oeval is TO TEST...")
	vm.oeval_loop = True
	while vm.oeval_loop:
	    vm.readexec(EAlisOpcodeKind.OPERAND)

# 0x17f0c: ofin (0x1d)
def ofin(vm):
	vm.oeval_loop = False

# 0x12e84: cnul (0x1e)
def cnul(vm):
	print("cnul is MISSING...")

# 0x12e84: cnul (0x1f)
def cnul(vm):
	print("cnul is MISSING...")

# 0x17efc: opushacc (0x20)
def opushacc(vm):
	# print("opushacc is MISSING...")
	vm.acc.append(vm.vars.d7)

# 0x17898: oand (0x21)
def oand(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 &= vm.vars.d6

# 0x178a0: oor (0x22)
def oor(vm):
	print("oor is MISSING...")

# 0x178a8: oxor (0x23)
def oxor(vm):
	print("oxor is MISSING...")

# 0x178b0: oeqv (0x24)
def oeqv(vm):
	print("oeqv is MISSING...")

# 0x178ba: oegal (0x25)
def oegal(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 = 0xffff if (vm.vars.d6 == vm.vars.d7) else 0x0

# 0x178ce: odiff (0x26)
def odiff(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 = 0xffff if (vm.vars.d6 != vm.vars.d7) else 0x0

# 0x178e2: oinfeg (0x27)
def oinfeg(vm):
	print("oinfeg is MISSING...")

# 0x178f6: osupeg (0x28)
def osupeg(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 = 0xffff if (vm.vars.d6 >= vm.vars.d7) else 0x0

# 0x1790a: oinf (0x29)
def oinf(vm):
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 = 0xffff if (vm.vars.d6 < vm.vars.d7) else 0x0

# 0x1791e: osup (0x2a)
def osup(vm):
	# print("osup is TO TEST...")
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d7 = 0xffff if (vm.vars.d6 > vm.vars.d7) else 0x0

# 0x17932: oadd (0x2b)
def oadd(vm):
	print("oadd is MISSING...")

# 0x1793a: osub (0x2c)
def osub(vm):
	print("osub is MISSING...")

# 0x17944: omod (0x2d)
def omod(vm):
	# print("omod is TO TEST...")
	vm.vars.d6 = vm.vars.d7
	vm.readexec(EAlisOpcodeKind.OPERAND)
	vm.vars.d6 %= vm.vars.d7
	vm.vars.d7 = vm.vars.d6

# 0x17952: odiv (0x2e)
def odiv(vm):
	print("odiv is MISSING...")

# 0x1795e: omul (0x2f)
def omul(vm):
	print("omul is MISSING...")

# 0x17966: oneg (0x30)
def oneg(vm):
	print("oneg is MISSING...")

# 0x1796a: oabs (0x31)
def oabs(vm):
	print("oabs is MISSING...")

# 0x17974: ornd (0x32)
def ornd(vm):
	print("ornd is MISSING...")

# 0x17996: osgn (0x33)
def osgn(vm):
	print("osgn is MISSING...")

# 0x179b0: onot (0x34)
def onot(vm):
	print("onot is MISSING...")

# 0x179b4: oinkey (0x35)
def oinkey(vm):
	print("oinkey is MISSING...")

# 0x179de: okeyon (0x36)
def okeyon(vm):
	print("okeyon is MISSING...")

# 0x17a06: ojoy (0x37)
def ojoy(vm):
	print("ojoy is MISSING...")

# 0x17a3e: oprnd (0x38)
def oprnd(vm):
	print("oprnd is MISSING...")

# 0x16150: oscan (0x39)
def oscan(vm):
	print("oscan is MISSING...")

# 0x179e0: oshiftkey (0x3a)
def oshiftkey(vm):
	print("oshiftkey is STUBBED...")
	if vm.vars.b_automode == 0:
		vm.vars.d7 = vm.sys.shiftkey()
	else:
		# TODO: shuff to do w/ d1 (value==6) but what ??
		pass

# 0x17a4c: ofree (0x3b)
def ofree(vm):
	print("ofree is STUBBED...")
	if vm.vars.d7 == 0:
		# TODO: return free program memory (finmem - finprog) in kB
		pass
	elif vm.vars.d7 == 1:
		# TODO: return free memory (debsprit - finent) in kB
		pass
	elif vm.vars.d7 == 2:
		# TODO: return remaining free script slots (maxprog - nbprog)
		pass
	elif vm.vars.d7 == 3:
		# TODO: return remaining free ent slots (maxent - nbent)
		pass
	# TODO: etc...
	# SIMULATED
	vm.vars.d7 = 0x321

# 0x17b16: omodel (0x3c)
def omodel(vm):
	vm.vars.d7 = vm.sys.get_model()

# 0x179fc: ogetkey (0x3d)
def ogetkey(vm):
	print("ogetkey is MISSING...")

# 0x17b20: oleft (0x3e)
def oleft(vm):
	print("oleft is MISSING...")

# 0x17b34: oright (0x3f)
def oright(vm):
	print("oright is MISSING...")

# 0x17b66: omid (0x40)
def omid(vm):
	print("omid is MISSING...")

# 0x17b8a: olen (0x41)
def olen(vm):
	print("olen is MISSING...")

# 0x17c3c: oasc (0x42)
def oasc(vm):
	print("oasc is MISSING...")

# 0x17c48: ostr (0x43)
def ostr(vm):
	print("ostr is MISSING...")

# 0x17ec6: osadd (0x44)
def osadd(vm):
	print("osadd is MISSING...")

# 0x17e42: osegal (0x45)
def osegal(vm):
	print("osegal is MISSING...")

# 0x17e58: osdiff (0x46)
def osdiff(vm):
	print("osdiff is MISSING...")

# 0x17e6e: osinfeg (0x47)
def osinfeg(vm):
	print("osinfeg is MISSING...")

# 0x17e84: ossupeg (0x48)
def ossupeg(vm):
	print("ossupeg is MISSING...")

# 0x17e9a: osinf (0x49)
def osinf(vm):
	print("osinf is MISSING...")

# 0x17eb0: ossup (0x4a)
def ossup(vm):
	print("ossup is MISSING...")

# 0x17e20: ospushacc (0x4b)
def ospushacc(vm):
	print("ospushacc is MISSING...")

# 0x17e26: ospile (0x4c)
def ospile(vm):
	print("ospile is MISSING...")

# 0x17ba2: oval (0x4d)
def oval(vm):
	print("oval is MISSING...")

# 0x17c62: oexistf (0x4e)
def oexistf(vm):
	f = bytearray(vm.vars.sd7)[:-1].decode()
	vm.vars.d7 = 0xffff if vm.sys.fexists(f) else 0

# 0x17c56: ochr (0x4f)
def ochr(vm):
	print("ochr is MISSING...")

# 0x17c7c: ochange (0x50)
def ochange(vm):
	print("ochange is MISSING...")

# 0x17d26: ocountry (0x51)
def ocountry(vm):
	print("ocountry is MISSING...")

# 0x17d30: omip (0x52)
def omip(vm):
	print("omip is MISSING...")

# 0x17a22: ojoykey (0x53)
def ojoykey(vm):
	print("ojoykey is MISSING...")

# 0x17d3a: oconfig (0x54)
def oconfig(vm):
	print("oconfig is MISSING...")


# =============================================================================
# STORENAME
# =============================================================================
# 0x12f2e: cnul (0x0)
def cnul(vm):
	print("cnul is MISSING...")

# 0x12f2e: cnul (0x1)
def cnul(vm):
	print("cnul is MISSING...")

# 0x12f2e: cnul (0x2)
def cnul(vm):
	print("cnul is MISSING...")

# 0x17f10: slocb (0x3)
# brief reads a word (offset) from script, then stores d7 byte at (vram + offset)
def slocb(vm):
	# print("slocb is TO TEST...")
	offset = vm.script.cread(2)
	vm.ram.write(offset, vm.vars.d7, 1)

# 0x17f1c: slocw (0x4)
def slocw(vm):
	# print("slocw is TO TEST...")
	offset = vm.script.cread(2)
	vm.ram.write(offset, vm.vars.d7, 2)

# 0x17f28: slocp (0x5)
def slocp(vm):
	offset = vm.script.cread(2)
	# TODO: it's basemain+offset !!
	vm.ram.writep(offset, vm.vars.oldsd7)

# 0x17f64: sloctp (0x6)
def sloctp(vm):
	print("sloctp is MISSING...")

# 0x17f40: sloctc (0x7)
# Store at LOCation with offseT: Char
def sloctc(vm):
	# print("sloctc is TO TEST...")
	offset = vm.script.cread(2)
	offset = tab_char(vm, offset)
	vm.vars.d7 = vm.acc.pop()
	# vm.ram.write(offset, vm.vars.d7, 1)

# 0x17f52: slocti (0x8)
def slocti(vm):
	print("slocti is MISSING...")

# 0x17f80: sdirb (0x9)
def sdirb(vm):
	print("sdirb is MISSING...")

# 0x17f8a: sdirw (0xa)
def sdirw(vm):
	print("sdirw is MISSING...")

# 0x17f94: sdirp (0xb)
def sdirp(vm):
	print("sdirp is MISSING...")

# 0x17fca: sdirtp (0xc)
def sdirtp(vm):
	print("sdirtp is MISSING...")

# 0x17faa: sdirtc (0xd)
def sdirtc(vm):
	print("sdirtc is MISSING...")

# 0x17fba: sdirti (0xe)
def sdirti(vm):
	print("sdirti is MISSING...")

# 0x17fe4: smainb (0xf)
def smainb(vm):
	print("smainb is MISSING...")

# 0x17ff6: smainw (0x10)
def smainw(vm):
	print("smainw is MISSING...")

# 0x18008: smainp (0x11)
def smainp(vm):
	print("smainp is MISSING...")

# 0x1805e: smaintp (0x12)
def smaintp(vm):
	print("smaintp is MISSING...")

# 0x18026: smaintc (0x13)
def smaintc(vm):
	print("smaintc is MISSING...")

# 0x18042: smainti (0x14)
def smainti(vm):
	print("smainti is MISSING...")

# 0x18084: shimb (0x15)
def shimb(vm):
	print("shimb is MISSING...")

# 0x180a4: shimw (0x16)
def shimw(vm):
	print("shimw is MISSING...")

# 0x180c4: shimp (0x17)
def shimp(vm):
	print("shimp is MISSING...")

# 0x18144: shimtp (0x18)
def shimtp(vm):
	print("shimtp is MISSING...")

# 0x180f0: shimtc (0x19)
def shimtc(vm):
	print("shimtc is MISSING...")

# 0x1811a: shimti (0x1a)
def shimti(vm):
	print("shimti is MISSING...")

# 0x18178: spile (0x1b)
def spile(vm):
	print("spile is MISSING...")

# 0x1817c: seval (0x1c)
def seval(vm):
	# print("seval is TO TEST...")
	# save d7 to virtual accumulator
	vm.acc.append(vm.vars.d7)
	oeval(vm)
	vm.readexec(EAlisOpcodeKind.STORENAME)

# 0x17f0c: ofin (0x1d)
# def ofin(vm):
# 	print("ofin is MISSING...")


# =============================================================================
# ADDNAME
# =============================================================================
# 0x12f6a: cnul (0x0)
def cnul(vm):
	print("cnul is MISSING...")

# 0x12f6a: cnul (0x1)
def cnul(vm):
	print("cnul is MISSING...")

# 0x12f6a: cnul (0x2)
def cnul(vm):
	print("cnul is MISSING...")

# 0x18194: alocb (0x3)
def alocb(vm):
	print("alocb is MISSING...")

# 0x181a0: alocw (0x4)
def alocw(vm):
	print("alocw is MISSING...")

# 0x181ac: alocp (0x5)
def alocp(vm):
	print("alocp is MISSING...")

# 0x181f0: aloctp (0x6)
def aloctp(vm):
	print("aloctp is MISSING...")

# 0x181cc: aloctc (0x7)
def aloctc(vm):
	print("aloctc is MISSING...")

# 0x181de: alocti (0x8)
def alocti(vm):
	print("alocti is MISSING...")

# 0x18214: adirb (0x9)
def adirb(vm):
	print("adirb is MISSING...")

# 0x1821e: adirw (0xa)
def adirw(vm):
	print("adirw is MISSING...")

# 0x18228: adirp (0xb)
def adirp(vm):
	print("adirp is MISSING...")

# 0x18266: adirtp (0xc)
def adirtp(vm):
	print("adirtp is MISSING...")

# 0x18246: adirtc (0xd)
def adirtc(vm):
	print("adirtc is MISSING...")

# 0x18256: adirti (0xe)
def adirti(vm):
	print("adirti is MISSING...")

# 0x18288: amainb (0xf)
def amainb(vm):
	print("amainb is MISSING...")

# 0x1829a: amainw (0x10)
def amainw(vm):
	print("amainw is MISSING...")

# 0x182ac: amainp (0x11)
def amainp(vm):
	print("amainp is MISSING...")

# 0x1830a: amaintp (0x12)
def amaintp(vm):
	print("amaintp is MISSING...")

# 0x182d2: amaintc (0x13)
def amaintc(vm):
	print("amaintc is MISSING...")

# 0x182ee: amainti (0x14)
def amainti(vm):
	print("amainti is MISSING...")

# 0x18338: ahimb (0x15)
def ahimb(vm):
	print("ahimb is MISSING...")

# 0x18358: ahimw (0x16)
def ahimw(vm):
	print("ahimw is MISSING...")

# 0x18378: ahimp (0x17)
def ahimp(vm):
	print("ahimp is MISSING...")

# 0x18400: ahimtp (0x18)
def ahimtp(vm):
	print("ahimtp is MISSING...")

# 0x183ac: ahimtc (0x19)
def ahimtc(vm):
	print("ahimtc is MISSING...")

# 0x183d6: ahimti (0x1a)
def ahimti(vm):
	print("ahimti is MISSING...")

# 0x18178: spile (0x1b)
def spile(vm):
	print("spile is MISSING...")

# 0x1843c: aeval (0x1c)
def aeval(vm):
	print("aeval is MISSING...")

# 0x17f0c: ofin (0x1d)
# def ofin(vm):
# 	print("ofin is MISSING...")


# =============================================================================
# COMMON STUFF & UTILS
# =============================================================================

def cbz(vm, offset):
	if vm.vars.d7 == 0:
		vm.script.jump(offset)

def cjmp(vm, offset):
	vm.script.jump(offset)

def cjsr(vm, offset):
    # // save return **OFFSET**, not ADDRESS
	vm.acc.append(vm.script.pc)
	vm.script.jump(offset)

def cstore_continue(vm):
	# swap chunk 1 / 3
	tmp = vm.vars.sd7
	vm.vars.sd7 = vm.vars.oldsd7
	vm.vars.oldsd7 = tmp    
	vm.readexec(EAlisOpcodeKind.STORENAME)

# 0x17818: tabchar
def tab_char(vm, offset):
#                      *******************************************************
#                      *                      FUNCTION                       *
#                      *******************************************************
#                      short __stdcall tabchar(void)
#        short           D0w:2        <RETURN>
#        undefined2      D0w:2        offset
#                      tabchar                                   XREF[12]:  000175e8(c), 00017650(c), 
#                                                                            000176d6(c), 000177ae(c), 
#                                                                            00017f46(c), 00017fae(c), 
#                                                                            00018034(c), 0001810c(c), 
#                                                                            000181d2(c), 0001824a(c), 
#                                                                            000182e0(c), 000183c8(c)  
#   00017818 12 36 00      move.b    (-0x1,A6,D0w*0x1),D1b
#            ff
#   0001781c 48 81         ext.w     D1w
	d1w = vm.ram.read(offset - 1, 1, True)

#   0001781e 41 f6 00      lea       (__DAT_0000fffe,A6,offset*0x1),A0
#            fe
	a0 = vm.ram.org + offset - 2 # DAT_0000fffe

#   00017822 d0 47         add.w     D7w,offset
#   00017824 51 c9 00      dbf       D1w,__loop
#            04
#   00017828 4e 75         rts
#                      __loop                                    XREF[2]:   00017824(j), 00017830(j)  
#   0001782a 34 1c         move.w    (A4)+,D2w
#   0001782c c5 e0         muls.w    -(A0),D2
#   0001782e d0 42         add.w     D2w,offset
#   00017830 51 c9 ff      dbf       D1w,__loop
#            f8
#   00017834 4e 75         rts
	pass

def scbreak(vm):
	print("scbreak is STUBBED..")

def scadd(vm):
	print("scadd is STUBBED..")