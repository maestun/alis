from array import array
from dataclasses import dataclass
from enum import Enum
import os

# =============================================================================
class EAlisOpcodeKind(str, Enum):
    OPCODE = "opcode"
    OPERAND = "operand"
    STORENAME = "storename"
    ADDNAME = "addname"


# =============================================================================
class AlisOpcode():
    def __init__(self, code: int, fptr, desc, addr):
        self.code = code
        self.fptr = fptr
        self.name = fptr.__name__
        self.desc = desc
        self.addr = addr


# =============================================================================
class AlisMemory():
    def __init__(self, org: int, size: int, is_le: bool, name: str, bytes = []):
        self.endian = "little" if is_le else "big"
        self.org = org
        if len(bytes) == 0:
            self.__data = [0] * size
        else:
            self.__data = bytes
        self.__name = name

    def read(self, offset: int, size: int, extend: bool = False):
        val = int.from_bytes(self.__data[offset: offset + size], self.endian)
        if extend:
            bit = 7 if size == 1 else 15
            if ((val >> bit) & 1):
                mask = 0xff00 if size == 1 else 0xffffff00
                val |= mask

        # print(f"read {size} bytes from {self.__name} at offset {hex(offset)}: {hex(val)}")
        return val
    
    def readp(self, offset: int):
        ret = []
        idx = 0
        c = self.__data[offset]
        while c != 0:
            ret.append(c)
            idx += 1
            c = self.__data[offset + idx]
        ret.append(c)
        return ret

    def write(self, offset: int, value: int, size: int = 1):
        bytes = int.to_bytes(value, size, self.endian)
        src_idx = 0
        for dst_idx in range(offset, offset + size):
            self.__data[dst_idx] = bytes[src_idx]
            src_idx += 1
        plural = ("s" if size > 1 else "")
        # print(f"wrote {size} byte{plural} ({hex(value)}) into {self.__name} at offset {hex(offset)}")

    def writep(self, offset, value):
        src_idx = 0
        for dst_idx in range(offset, offset + len(value)):
            self.__data[dst_idx] = value[src_idx]
            src_idx += 1


# =============================================================================
class AlisScriptHeader():
    HEADER_SZ = 24
    def __init__(self, f, is_le):
        endian = "little" if is_le else "big"
        self.id = int.from_bytes(f.read(2), endian)
        # seems to be always 0x1700 on atari, copied at (vram - $2e)
        self.w_0x1700 = int.from_bytes(f.read(2), endian)
        # where does code start from (script header + id) ? on atari, always $16
        self.code_loc_offset = int.from_bytes(f.read(2), endian)
        # some scripts have a "sub-script" into them: this is the offset to their code location
        self.ret_offset = int.from_bytes(f.read(4), endian)
        # TODO: ???
        self.dw_unknown3 = int.from_bytes(f.read(4), endian)
        # TODO: ???
        self.dw_unknown4 = int.from_bytes(f.read(4), endian)
        # almost always 0x20 (is 0xa for "message*" and 0x4 for "objet")
        self.w_unknown5 = int.from_bytes(f.read(2), endian)
        # number of bytes to alloc fo this script
        self.vram_alloc_sz = int.from_bytes(f.read(2), endian)
        # almost always 0x20 (is 0x4 for "message*" and "objet")
        self.w_unknown7 = int.from_bytes(f.read(2), endian)


# =============================================================================
# TODO: inherit from AlisMemory, to get std read and write methods
class AlisScript():
    def __init__(self, path: str, is_le: bool):
        with open(path, "rb") as f:
            self.name = os.path.basename(path).split(".")[0]
            # read HEADER_SZ bytes
            self.header = AlisScriptHeader(f, is_le)
            # remaining bytes are script code & data
            self.size = os.stat(path).st_size - AlisScriptHeader.HEADER_SZ
            self.code = AlisMemory(0, self.size, is_le, self.name + "-code", f.read())
            f.close()
            # TODO: alloc virtual ram needed ?
            self.vram = AlisMemory(0, self.header.vram_alloc_sz, is_le, self.name + "-vram")
            # set program counter to 1st script byte
            self.pc = 0
            self.is_running = False
            self.stop = False

    def cread(self, size: int = 1, extend: bool = False):
        if size > 0:
            val = self.code.read(self.pc, size, extend)
            self.pc += size
            return val
        else:
            val = self.code.readp(self.pc)
            self.pc += len(val)
            return val

    def vread(self, offset: int, size: int = 1, extend: bool = False):
        return self.vram.read(offset, size, extend)

    def vwrite(self, offset: int, value: int, size: int = 1):
        self.vram.write(offset, value, size)

    def jump(self, offset, is_offset = True):
        """Relocates or moves this script's Program Counter

        Args:
            offset (int): jump value
            is_offset (bool, optional): if true, value is added to PC, else PC is relocated. Defaults to True.
        """
        self.pc = offset + (self.pc if is_offset else 0)


# =============================================================================
@dataclass
class AlisVars():
    d6 = 0
    d7 = 0
    sd6 = [0] * 256
    sd7 = [0] * 256
    oldsd7 = [0] * 256

    # TODO: compute from packed main script header
    addr_basemain = 0x22690

    b_automode = 0
    b_timing = 0
    b_tvmode = 0

    w_cx = 0
    w_cy = 0
    w_cz = 0
    w_crnd = 0
    w_fswitch = 0


# =============================================================================
@dataclass
class AlisContext():
   _0x1c_scan_clr = 0
   _0x1e_scan_clr = 0
   _0x22_cscreen = 0
   _0x24_scan_off_bit_0 = False
   _0x24_scan_clr_bit_7 = False
   _0x24_inter_off_bit_1 = False
   