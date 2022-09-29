from enum import Enum
import os

kMainScript = "main"

# =============================================================================
class HostPlatform():
    def __init__(self, name, ram, baseram, extension, width, height, bpp, is_little_endian):
        self.name = name
        self.extension = extension
        self.ram = ram
        self.baseram = baseram
        self.width = width
        self.height = height
        self.bpp = bpp
        self.is_le = is_little_endian

    def set_path(self, path):
        self.path = path
        self.main = path + os.path.sep + "main" + "." + self.extension


# =============================================================================
class EHostPlatform(Enum):
    Atari =     HostPlatform("Atari ST/STe", 0x100000, 0x22400, "ao", 320, 200, 5, False)
    Falcon =    HostPlatform("Atari Falcon", 0x400000, 0,       "fo", 320, 200, 8, False)
    Amiga =     HostPlatform("Amiga",        0x100000, 0,       "co", 320, 200, 5, False)
    AmigaAGA =  HostPlatform("Amiga AGA",    0x200000, 0,       "do", 320, 200, 8, False)
    Mac =       HostPlatform("Macintosh",    0x400000, 0,       "mo", 320, 200, 5, False)
    PC =        HostPlatform("MS/DOS",       0x100000, 0,       "io", 320, 200, 8, True)
    Unknown =   HostPlatform("Unknown",      0,        0,       "??",   0,   0, 0, True)

    def guess(path) -> HostPlatform:
        if os.path.isdir(os.path.join(os.getcwd(), path)):
            # get main script
            for file in os.listdir(path):
                file = file.lower()
                if file.startswith(kMainScript):
                    ext = file.split(".")[-1]
                    for pl in EHostPlatform:
                        if pl.value.extension == ext:
                            return pl
        
        return EHostPlatform.Unknown.value

