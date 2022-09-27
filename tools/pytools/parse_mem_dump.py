from fileinput import close
import os;

DUMP_FILE = "/Users/olivier/dev/atari/alis/tools/Steem.SSE.4.0.2.Debug.Win64.DD/memory snapshots/presen.dump"
ENDIAN = "big"

SCRIPTS_FOLDER = "/Users/olivier/dev/atari/alis/data/ishar/atari/decrunched/"
ADDR_BASEMEM = 0x22400

scripts = {}
script_files = [f for f in os.listdir(SCRIPTS_FOLDER) if (os.path.isfile(os.path.join(SCRIPTS_FOLDER, f)) and f.endswith(".alis"))]
for script_file in script_files:
    with open(os.path.join(SCRIPTS_FOLDER, script_file), "rb") as f:
        id = int.from_bytes(f.read(2), ENDIAN)
        scripts[id] = script_file

with open(DUMP_FILE, "rb") as fdump:
    dump = fdump.read()
    fdump.close()

    # extract loaded scripts
    nb_prog = int.from_bytes(dump[0x1953e: 0x1953e+2], ENDIAN)
    addr_atprog = int.from_bytes(dump[0x19530: 0x19530+4], ENDIAN)
    
    for idxprog in range(0, nb_prog):
        addr = addr_atprog + (4 * idxprog)
        prog_addr = int.from_bytes(dump[addr: addr + 4], ENDIAN)
        prog_id = int.from_bytes(dump[prog_addr: prog_addr + 2], ENDIAN)
        prog_name = scripts[prog_id]
        print(f"Found {prog_name} (id = {hex(prog_id)}) at addr {hex(prog_addr)}")

    # extract ents
    nb_ent = int.from_bytes(dump[0x19552: 0x19552+2], ENDIAN)
    addr_atent = int.from_bytes(dump[0x19544: 0x19544+4], ENDIAN)
    for idxent in range(0, nb_ent):
        addr = addr_atent + (6 * idxent)
        ent_addr = int.from_bytes(dump[addr: addr + 4], ENDIAN)
        ent_offset = int.from_bytes(dump[addr + 4: addr + 6], ENDIAN)
        print(f"Found ent {idxent} offset = {hex(ent_offset)}, addr {hex(ent_addr)}")



print("done")