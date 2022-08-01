# ALIS - Actor Language Integrated System

**WORK IN PROGRESS**

An attempt to reimplement the virtual machine used by the Silmarils game company.

## Build and run

```bash
mkdir build && cd build
cmake ..
```

On case-sensitive systems you must rename all data files to lowercase:

```bash
sudo apt install rename
cd data_path
rename 'y/A-Z/a-z/' *
```


Run the executable by passing the data path.

```bash
./alis data_path
```

## The beginning

Everything started while checking the files present on the Silmarils Trilogy CD-ROM that I bought a long time ago.

Those games ran on a custom DOSBox engine, and since this project has a GPL license the source files were present.

Some of those caught my attention, they were not part of the DOSBox sources, and looked like custom opcode tables.

Since Silmarils was a company producing several games for all platforms of the 90's at a very fast pace, I thought
that they were probably running on a custom virtual machine of some kind ([they did](https://www.youtube.com/watch?v=TKAg3JMLXzM)). 

Time to reverse engineer !

Since the Atari ST is the machine I grew up with (and its 68000 CPU is the easiest to understand IMHO), I decided to try with this version.

## The tools

- Ubuntu Linux machine, with wine installed
- Steem SSE Debugger 4.0.2
- Pasti DLL
- Ghidra 10+
- Some custom python scripts

## The Virtual Machine (VM)

### The basics

On the game disk, we'll find the main executable, and several compressed game files. These games files contain almost all of the game mechanics, the assets (sounds, graphics, messages, etc...), as well as the VM's code in a proprietary pseudo-assembly format.

The main executable contains:
- all the system calls, that depend on the target's architecture
- all the VM's opcodes. 
- some jump (or lookup) tables, that are used to jump directly to an opcode implementation without the need for switch/case statements (optimization !)

When loaded, the main executable initializes the target, loads and decompresses the main script file ("main.ao"), then start the VM's main loop which fetches the opcodes read from the decompressed script.

### VM architecture



### Error codes

- 0x1: failed to open file
- 0x2:
- 0x3:
- 0x4:
- 0x5:
- 0x6:
- 0x7: failed to write to file
- 0x8: failed to create file
- 0x9: failed to delete file
- 0xa: create element error
- 0xb:
- 0xc:
- 0xd: failed to read from file
- 0xe: failed to close file
- 0xf:
- 0x10: error clipform / mat


### Project structure

./alis
VM source code

./data
Data files to run with VM

./ghidra
Ghidra projets for reversing

./imgui
Dear ImGui source code

./macOS
Platform dependencies

./tools
Other tools for reversing
On macOS Catalina run the atari emulator with 'wine64 Debugger64DD.exe NODD'

