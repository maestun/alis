# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ALIS is a reimplementation of the virtual machine (VM) known as ALIS (Actor Language Integrated System), created by the Silmarils videogame company. This VM was used to run numerous adventure and RPG games from the 1990s across multiple platforms including PC, Atari ST, Amiga, and others.

## Build Commands

### Primary Build (CMake - Recommended)
```bash
# Standard build
mkdir build && cd build
cmake ..
make

# Clean build
rm -rf build && mkdir build && cd build && cmake .. && make
```

### Alternative Build (Make)
```bash
# Standard build using Makefile
make

# Clean
make clean
```

### Atari ST Cross-compilation
```bash
# Use Atari-specific Makefile
make -f "Makefile Atari"
```

### Running
```bash
# Basic usage
./alis <data_path>

# Fullscreen mode
./alis -f <data_path>

# Mute sound
./alis -m <data_path>

# Enable runtime disassembler
./alis -d <data_path>

# Unpack mode
./alis -u <data_path>
```

## Dependencies

- CMake (for primary build system)
- SDL2 (graphics and audio)
- Cross-compilation toolchain for Atari ST builds (m68k-atari-mint-gcc)

## Architecture Overview

### Core Components

- **Virtual Machine Core** (`src/alis.c`, `src/alis.h`): Main VM implementation with opcode execution
- **Script System** (`src/script.c`, `src/script.h`): Script loading, decompression, and management
- **Memory Management** (`src/mem.c`, `src/mem.h`): VM memory allocation and management
- **Image/Graphics** (`src/image.c`, `src/image.h`): Sprite and graphics rendering system
- **Video System** (`src/video.c`, `src/screen.c`): Display management and rendering pipeline
- **Audio System** (`src/audio/`): Music and sound effect playback
- **Platform Abstraction** (`src/platform.c`, `src/sys/sys.c`): Platform-specific implementations
- **Opcodes** (`src/opcodes.c`): VM instruction implementations
- **Unpacking** (`src/unpack.c`): Game file decompression utilities

### Audio Emulation
- **YM2149** (`src/emu2149/`): Atari ST sound chip emulation
- **YM2413** (`src/emu2413/`): MSX sound chip emulation for enhanced audio

### Key Data Structures
- `sAlisVM alis`: Main VM state structure
- `sScript`: Script management structure
- `sSprite`: Sprite rendering data
- Script execution happens in a main loop that fetches and executes opcodes

### Game Support
The VM supports 30+ Silmarils games with varying levels of compatibility. Game data files are typically compressed and contain:
- Game logic in proprietary pseudo-assembly format
- Graphics assets (sprites, backgrounds)
- Audio resources (music, sound effects)
- Text and localization data

### Debug Features
- Runtime disassembler (`-d` flag) for script analysis
- Savestate system (F11/F12 keys)
- Built-in unpacking utilities for analyzing game files
- Debug output and logging system

### Platform Support
- Modern systems via SDL2 (Windows, macOS, Linux)
- Atari ST via cross-compilation
- Multiple compiler support (GCC, MSVC, MinGW, Cygwin)

## Development Notes

- The codebase uses C99 standard
- Platform-specific code is isolated in `src/sys/` and `src/platform.c`
- Game compatibility status is tracked in README.md
- Known issues and ongoing work are documented in TODO file
- The project maintains compatibility with original game data files
- Extensive reverse engineering work has been done to understand the original VM behavior