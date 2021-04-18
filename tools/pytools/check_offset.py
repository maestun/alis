import glob
import os

def list_offsets(folder):
    files = glob.glob(folder)
    ids = {}

    print("FILENAME;FILESIZE;ID;WORD 1;WORD 2;OFFSET;DWORD 3;DWORD 4;WORD 5;WORD 6;WORD 7")

    for file in files:
        f = open(file, "rb")
        file_size = os.path.getsize(file)
        bytes = f.read()

        # word: script ID
        id = (bytes[0] << 8) + bytes[1]

        # word: unknown
        uw1 = (bytes[2] << 8) + bytes[3]

        # word: unknown
        uw2 = (bytes[4] << 8) + bytes[5]

        # dword: return offset
        offset = (bytes[6] << 24) + (bytes[7] << 16) + (bytes[8] << 8) + bytes[9]

        # dword: unknown
        udw1 = (bytes[10] << 24) + (bytes[11] << 16) + (bytes[12] << 8) + bytes[13]

        # dword: unknown
        udw2 = (bytes[14] << 24) + (bytes[15] << 16) + (bytes[16] << 8) + bytes[17]

        # word: unknown
        uw3 = (bytes[18] << 8) + bytes[19]

        # word: unknown
        uw4 = (bytes[20] << 8) + bytes[21]

        # word: unknown
        uw5 = (bytes[22] << 8) + bytes[23]

        ids[id] = file

        comps = file.split('/')
        print(comps[len(comps) - 1] + ";" + hex(file_size) + ";" + hex(id) + ";" + hex(uw1) + ";" + hex(uw2) +
              ";" + hex(offset) + ";" + hex(udw1) + ";" + hex(udw2) + ";" + hex(uw3) + ";" + hex(uw4) + ";" + hex(uw5))
        f.close()

list_offsets("/Users/developer/dev/perso/macos/alis/data/ishar/atari/depack/*.AO")