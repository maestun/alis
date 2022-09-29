from alis_vm import AlisVM, vm
from alis_defs import EAlisOpcodeKind



BP_STR = "!!!! PASSED BREAKPOINT at address "
LOG_FILE_PATH = "./tools/Steem.SSE.4.0.2.Debug.Win64.DD/steem.log"
OUT_FILE_PATH = "./tools/Steem.SSE.4.0.2.Debug.Win64.DD/startup_to_logo_opcodes.csv"
OPCODE_READ_ADDR = 0x01310c
OPERAND_READ_ADDR = 0x017572
STORENAME_READ_ADDR_1 = 0x013406 # in storeacc0
STORENAME_READ_ADDR_2 = 0x01818a # in STORE_SEVAL
ADDNAME_READ_ADDR_1 = 0x013438 # in addacc0
ADDNAME_READ_ADDR_2 = 0x01844a # in ADD_AEVAL
SYS_FOPEN_ADDR = 0x00c10e


# -----------------------------------------------------------------------------
class SteemLogBreakpoint():
    def __init__(self, lines):
        # parse breakpoint log from a pack of 4 lines similar to:
        #
        # !!!! PASSED BREAKPOINT at address $017572, sr=2308
        # d0=00006a  d1=000064  d2=030000  d3=000042  d4=ffffa8  d5=000000  d6=000000  d7=000000  
        # a0=02d278  a1=004dc9  a2=012cb6  a3=03210d  a4=0198de  a5=012e84  a6=022690  a7=0f7fec  
        # time=822372320 scanline=-33 cycle=232

        data = lines[0].replace(BP_STR + "$", "").split(", sr=")
        self.addr = int(data[0], base=16)
        self.sr = int(data[1], base=16)
        self.dreg = []
        regs = lines[1].strip().split("  ")
        for reg in regs:
            self.dreg.append(int(reg.split("=")[1], base=16))
        self.areg = []
        regs = lines[2].strip().split("  ")
        for reg in regs:
            self.areg.append(int(reg.split("=")[1], base=16))
        data = lines[3].split(" ")
        self.time = int(data[0].replace("time=", ""))
        self.scanline = int(data[1].replace("scanline=", ""))
        self.cycle = int(data[2].replace("cycle=", ""))
# -----------------------------------------------------------------------------

with open(LOG_FILE_PATH, "r") as f: 
    log_lines = f.read().splitlines() 

disasm_lines = []
disasm_line = ""
idx = 0

# each bkp trace has 6 lines, sort
breakpoint_traces = {}
breakpoints = []
breakpoint_idx = 0

for i in range(0, len(log_lines)):
    log_line = log_lines[i]
    if log_line.startswith("!!!! PASSED BREAKPOINT"):
        # stack 4 lines
        breakpoint_traces[breakpoint_idx] = []
        breakpoint_traces[breakpoint_idx].append(log_lines[i])
        breakpoint_traces[breakpoint_idx].append(log_lines[i + 1])
        breakpoint_traces[breakpoint_idx].append(log_lines[i + 2])
        breakpoint_traces[breakpoint_idx].append(log_lines[i + 3])
        breakpoints.append(SteemLogBreakpoint(breakpoint_traces[breakpoint_idx]))
        breakpoint_idx += 1

opcode_idx = 0
disasm_lines.append("opcode_idx;script_addr;opcode_kind;opcode_name;opcode_code;opcode_addr\n")
for bp in breakpoints:
    if bp.addr == SYS_FOPEN_ADDR:
        addr = bp.areg[0]
        disasm_line = "-- FOPEN at " + hex(addr)

    if bp.addr == OPCODE_READ_ADDR:
        code = bp.dreg[0] & 0xff
        opcode = vm.opcodes[EAlisOpcodeKind.OPCODE][code]
        addr = bp.areg[3] - 1
        disasm_line = f"{opcode_idx};{hex(addr)};OPCODE;{opcode.name};{hex(opcode.code)};{hex(opcode.addr)}\n"
        opcode_idx += 1
    
    elif bp.addr == OPERAND_READ_ADDR:
        code = bp.dreg[0] & 0xff
        operand = vm.opcodes[EAlisOpcodeKind.OPERAND][code]
#        name = AlisVM.operand_names[code]
        addr = bp.areg[3] - 1
        disasm_line = f"-;{hex(addr)};OPERAND;{operand.name};{hex(operand.code)};{hex(operand.addr)}\n"
#        disasm_line +=  " " + name + " (" + hex(code) + ")"

    elif bp.addr == STORENAME_READ_ADDR_1 or bp.addr == STORENAME_READ_ADDR_2:
        code = bp.dreg[0] & 0xff
        name = AlisVM.store_names[code]
        addr = bp.areg[3] - 1
        disasm_line +=  " " + name + " (" + hex(code) + ")"

    elif bp.addr == ADDNAME_READ_ADDR_1 or bp.addr == ADDNAME_READ_ADDR_2:
        code = bp.dreg[0] & 0xff
        name = AlisVM.add_names[code]
        addr = bp.areg[3] - 1
        disasm_line +=  " " + name + " (" + hex(code) + ")"

    disasm_lines.append(disasm_line)

# loop 'manually'
# for idx in range(0, len(log_lines)):

#     log_line = log_lines[idx]
    
#     def get_register_value(kind: str, num: int):
#         inc = 1 if kind == "data" else 2
#         regs = log_lines[idx + inc].split("  ")
#         value = regs[num].split("=")
#         return value[1]


#     # TODO: DETECT LOAD SCRIPT FILE


#     # READ OPCODE
#     if log_line.startswith(BP_STR + OPCODE_READ_ADDR):
#         # idx += 1
#         # data_regs = log_lines[idx].split("  ")
#         # idx += 1
#         # addr_regs = log_lines[idx].split("  ")
        
#         # opcode is byte in d0 register
#         d0_hex_byte = get_register_value("data", 0)[4:]
        
#         # d0_hex_value = data_regs[0].split("=")[1]
#         opcode_byte = int(d0_hex_byte, base=16)
#         opcode_name = AlisVM.opcode_names[opcode_byte]

#         # opcode was read from (a3 - 1) (a3 register was incremented after reading opcode)
#         # a3_hex_value = addr_regs[3].split("=")[1]
#         a3_hex_value = get_register_value("addr", 3)
#         addr = int(a3_hex_value, base=16) - 1
#         disasm_line = hex(addr) + ": " + opcode_name + " (" + hex(opcode_byte) + ")"

#     # READ OPERAND
#     elif log_line.startswith(BP_STR + OPERAND_READ_ADDR):
#         d0_hex_byte = get_register_value("data", 0)[4:]
#         operand_byte = int(d0_hex_byte, base=16)
#         operand_name = AlisVM.operand_names[operand_byte]
#         # a3_hex_value = get_register_value("addr", 3)
#         # addr = int(a3_hex_value, base=16) - 1
#         disasm_line += " " + operand_name
    
#     # FOPEN ?
#     elif log_line.startswith(BP_STR + SYS_FOPEN_ADDR):
#         a0_hex_value = get_register_value("addr", 0)
#         disasm_line = "-- FOPEN at " + hex(int(a0_hex_value, base=16))

#     if disasm_line != "":
#         disasm_lines.append(disasm_line)

for disasm_line in disasm_lines:
    print(disasm_line)

with open(OUT_FILE_PATH, "w") as f:
    f.writelines(disasm_lines)
    f.close()

