

from enum import Enum, auto

import argparse
import os


kProgName   = "alis"
kProgVersion = "0.0.1"
kMainScript = "main"


# -----------------------------------------------------------------------------
class Platform():
    def __init__(self, name, extension, width, height, bpp, is_little_endian):
        self.name = name
        self.extension = extension
        self.width = width
        self.height = height
        self.bpp = bpp
        self.is_le = is_little_endian

    def set_path(self, path):
        self.path = path
        self.main = path + os.path.sep + "main" + "." + self.extension


# -----------------------------------------------------------------------------
class EPlatform(Enum):
    Atari =     Platform("Atari ST/STe", "ao", 320, 200, 5, False)
    Falcon =    Platform("Atari Falcon", "fo", 320, 200, 8, False)
    Amiga =     Platform("Amiga",        "co", 320, 200, 5, False)
    AmigaAGA =  Platform("Amiga AGA",    "do", 320, 200, 8, False)
    Mac =       Platform("Macintosh",    "mo", 320, 200, 5, False)
    PC =        Platform("MS/DOS",       "io", 320, 200, 8, True)
    Unknown =   Platform("Unknown",      "??",   0,   0, 0, True)

    def guess(path) -> Platform:
        if os.path.isdir(os.path.join(os.getcwd(), path)):
            # get main script
            for file in os.listdir(path):
                file = file.lower()
                if file.startswith(kMainScript):
                    ext = file.split(".")[-1]
                    for pl in EPlatform:
                        if pl.value.extension == ext:
                            return pl
        
        return EPlatform.Unknown.value



pl = EPlatform.guess("../hdd/ishar1_uncracked_daze/data")    
