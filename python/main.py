import os

from alis_vm import AlisVM, vm
from host import EHostPlatform

# =========================================================================
# kProgName = "alis"
# kGitHash = os.popen("git rev-parse --short HEAD").read()
# kGitTag = os.popen("git tag --points-at HEAD").read()
# kGitBranch = os.popen("git rev-parse --abbrev-ref HEAD").read()

# ALIS_NAME = "Ishar II / Atari / cracked by Elite"
# ALIS_DATA_PATH = "./data/ishar/atari/decrunched"
# ALIS_EXE_PATH = "./data/ishar2/atari/auto/ISHAR2OK.PRG"
# ALIS_EXE_MD5 = "87471ae02afacf5da303a99ce81ec1cd"

# vm = AlisVM(name=ALIS_NAME, 
#             exe_path=ALIS_EXE_PATH,
#             exe_md5=ALIS_EXE_MD5,
#             exe_addr=0xaa9a,
#             data_path=ALIS_DATA_PATH,
#             platform=EHostPlatform.Atari,
#             opcode_tab_addr=0x12cb6,
#             opername_tab_addr=0x12e84,
#             storename_tab_addr=0x12f2e,
#             addname_tab_addr=0x12f6a)

vm.run()