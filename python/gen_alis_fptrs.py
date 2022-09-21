# =============================================================================
# Chicken and Egg utility :)
#
# Generates the "./python/alis_fptrs.py" files that contains all the 
#   AlisVM opcodes functions, to be implemented later.
#
# Use this only once !!!
# =============================================================================
from host import EHostPlatform
from alis_vm import AlisVM, EAlisOpcodeKind
import datetime

ALIS_GAME = "Ishar II / Atari / cracked by Elite"
ALIS_EXE_PATH = "./data/ishar2/atari/auto/ISHAR2OK.PRG"
ALIS_EXE_MD5 = "87471ae02afacf5da303a99ce81ec1cd"
ALIS_DATA_PATH = "./data/ishar/atari/decrunched"

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
            addname_tab_addr=0x12f6a, 
            fake_opcodes=True)


def gen_python_fptrs(output_path):
    with open(output_path, "w") as f:
        now = datetime.datetime.now()
        code = "# " + ALIS_GAME + "\n"
        code += "# " + ALIS_EXE_PATH + "\n" 
        code += "# " + ALIS_EXE_MD5 + "\n"
        code += "# " + now.strftime("%Y-%m-%d %H:%M:%S") + "\n\n"
        # code += "from alis_vm import AlisVM\n\n"

        for kind in EAlisOpcodeKind:
            code += "# =============================================================================\n"
            code += "# " + kind.upper() + "\n"
            code += "# =============================================================================\n"
            for opcode in vm.opcodes[kind]:
                code += "# " + hex(opcode.addr) + ": " + opcode.desc + " (" + hex(opcode.code) + ")\n"
                code += "def " + opcode.desc + "(vm):\n\tprint(\"" + opcode.desc + " is MISSING...\")\n\n"
            code += "\n"
        f.write(code)
        f.close()


def gen_c_fptrs(output_path):
    with open(output_path, "w") as f:
        now = datetime.datetime.now()
        code = "// " + ALIS_GAME + "\n"
        code += "// " + ALIS_EXE_PATH + "\n" 
        code += "// " + ALIS_EXE_MD5 + "\n"
        code += "// " + now.strftime("%Y-%m-%d %H:%M:%S") + "\n\n"
        code += "#include \"alis.h\"\n\n"

        for kind in EAlisOpcodeKind:
            code += "// ============================================================================\n"
            code += "#pragma mark -  " + kind.upper() + "\n"
            code += "// ============================================================================\n"
            for opcode in vm.opcodes[kind]:
                code += "// " + hex(opcode.addr) + ": " + opcode.desc + " (" + hex(opcode.code) + ")\n"
                code += "static void " + opcode.desc + "(void) {\n\tprintf(\"" + opcode.desc + " is MISSING...\");\n}\n\n"
            code += "\n"
        f.write(code)
        f.close()

gen_python_fptrs("./python/alis_fptrs.py")
# gen_c_fptrs("./python/alis_fptrs.c")