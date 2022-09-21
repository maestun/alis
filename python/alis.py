
from alis_vm import AlisVM, EAlisOpcodeKind
from host import EHostPlatform

kProgName   = "alis"
kProgVersion = "0.0.1"

# pl = EHostPlatform.guess("../hdd/ishar1_uncracked_daze/data")    

ALIS_GAME = "Ishar II / Atari / cracked by Elite"
ALIS_DATA_PATH = "./data/ishar/atari/decrunched"
ALIS_EXE_PATH = "./data/ishar2/atari/auto/ISHAR2OK.PRG"
ALIS_EXE_MD5 = "87471ae02afacf5da303a99ce81ec1cd"

vm = AlisVM(name=ALIS_GAME, 
            exe_path=ALIS_EXE_PATH,
            exe_md5=ALIS_EXE_MD5,
            exe_addr=0xaa9a,
            data_path=ALIS_DATA_PATH,
            platform=EHostPlatform.Atari,
            vram_addr=0x22400,
            opcode_tab_addr=0x12cb6,
            opername_tab_addr=0x12e84,
            storename_tab_addr=0x12f2e,
            addname_tab_addr=0x12f6a)

# for kind in EAlisOpcodeKind:
#     for opcode in vm.opcodes[kind]:
#         print(hex(opcode.addr) + ": " + opcode.name + " (" + hex(opcode.code) + "):")
#         opcode.fptr()

vm.run()