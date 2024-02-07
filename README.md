# ALIS

An attempt to reimplement the virtual machine made by the [Silmarils](https://en.wikipedia.org/wiki/Silmarils_(company)) videogame company. The name of this repo comes from [this interview](https://www.atarilegend.com/interviews/26).

## Build and run

Tested on macOS and Ubuntu.

Install the following packages on your system:

- CMake
- SDL2

```bash
mkdir build && cd build
cmake ..
make
```

You need to copy the original data files of the game you wish to run to a given folder.

On case-sensitive systems you must rename all data files to lowercase:

```bash
sudo apt install rename
cd data_path
rename 'y/A-Z/a-z/' *
```

Run the executable by passing the data path:

```bash
./alis data_path
```

## More info...

### What is working

| Game                   | MS DOS                                       | Atari ST                                     | Atari Falcon                                 | Amiga                                        | Amiga AGA                                    | Macintosh                                    | 3DO                                          | Amstrad CPC                                  |
| :---                   | :---                                         | :---                                         | :---                                         | :---                                         | :---                                         | :---                                         | :---                                         | :---                                         |
| Manhattan Dealers      | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Mad Show               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Windsurf Willy         | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\small\color{gray}Not \ Working}}$ |
| Targhan                | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\small\color{gray}Not \ Working}}$ |
| Xyphoes Fantasy        | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\small\color{gray}Not \ Working}}$ |
| Le Fetiche Maya        | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Colorado               | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Starblade              | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Storm Master           | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Metal Mutant           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Crystals Of Arborea    | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Transarctica           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Boston Bomb Club       | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Bunny Bricks           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Ishar 1                | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Ishar 2                | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{green}Playable}}$           | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Ishar 3                | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{orange}Bugged}}$            | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Robinson's Requiem     | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\color{red}Broken}}$               | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   |
| Asghan                 | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Deus                   | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Tournament Of Warriors | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |
| Time Warriors          | ${\textsf{\small\color{gray}Not \ Working}}$ | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   | ${\textsf{\color{gray}-}}$                   |

- Not Working, main script is not executed at all.               
- Broken, game is executed but crashes or freezes shortly after execution.               
- Bugged, partially playable, game breaking bugs are to be expected.
- Playable, at least partially tested with no major bugs found.
- Complete, fully playable and tested.

### Development tools used for reversing

- Ubuntu Linux
- Steem SSE Debugger
- Pasti DLL
- Ghidra
- Some custom python scripts

### Virtual Machine

On a Silmarils game disk, you'll find the main executable (the VM), and several compressed game files. These games files contain almost all of the game mechanics, the assets (sounds, graphics, messages, etc...), as well as the VM's code in a proprietary pseudo-assembly format.

The main executable contains:
- all the system calls, that depend on the target's architecture
- all the VM's opcodes. 
- some jump (or lookup) tables, that are used to jump directly to an opcode implementation without the need for switch/case statements (optimization !)

When loaded, the main executable initializes the target, loads and decompresses the main script file ("main.XX"), then start the VM's main loop which fetches the opcodes read from the decompressed script.

### Why ?

When I was a kid in the nineties, I loved playing the Ishar series of games (or at least the two first episodes). I ran those games on my good old Atari ST, and was amazed of good looking and atmosphere-filled they were.

Fast forward a good decade. I became a software developer, working with bloated'n'buggy enterprise software, and was super bored at work.
I loved playing on my Gameboy Advance console, and was also trying to code for it.
Hey, wouldn't it be cool to code a dungeon-crawling game like Ishar ? Oh, it'd be sooo cool to reuse those oldskool looking graphics !
Okay, time to crack the inner workings of this old game, that should be that diffic... oh.

So yeah, I quickly realized that my knownledge was far from sufficient to grab the inner workings of these games.
PC version, Atari version, Amiga version: none of the file formats were matching anything I knew of.
I tried to reverse-engineer the games but was quickly overwhelmed by the amount of work to do.

I ended up coding a little demo (which you can still find [here](https://pdroms.de/files/nintendo-gameboyadvance-gba/ishar-advance-v0-0-alpha)), made using screenshots of the PC version of Ishar 1 and 2, captured using the DOSBox emulator (!).
The demo didn't do much, you cound only crawl into a small dungeon and display the players' inventory.
Since I didn't know anything about version control or making backups, when my computer was stolen I lost all my precious source code, which had taken forever to write.

Sad ending, but at least:
- I learned low-level and bare-metal coding in C
- The demo embedded a primitive scripting system using .ini files, xml and lua (xml on a GBA... don't do that :)
- I showed the demo to a friend who was starting a small company into the mobile development (in 2008, so that scene was quite the hype), who then offered me a much better paying job, and so much fun too !

### Now

Fast forward many more years, I've gained some coding experience and decided to finally understand how this Ishar games were really working !

Luckily, when checking the files present on the Silmarils Trilogy CD-ROM that I bought a few years back, I ran into some C files that were not part of the DOSBox sources (but had to be shipped, do to the GPL license), and looked like custom opcode tables.

Since Silmarils was a company producing several games for all platforms of the 90's at a very fast pace, I thought that they were probably running on a custom virtual machine of some kind ([yep, they did](https://www.youtube.com/watch?v=TKAg3JMLXzM)).

Time to reverse engineer !
