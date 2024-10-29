//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the “Software”), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma once

#include "config.h"
#include "debug.h"
#include "platform.h"
#include "script.h"
#include "sys/sys.h"
#include "utils.h"

#ifndef max
# define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef _MSC_VER
# define PACK_ATTR
# define PACK_PUSH __pragma(pack(push, 1))
# define PACK_POP  __pragma(pack(pop))
#elif __GNUC__
# define PACK_ATTR __attribute__((__packed__))
# define PACK_PUSH
# define PACK_POP
#endif


extern const u32 kControlsTicks;
extern const u32 kFrameTicks;

extern const u32 kHostRAMSize;
extern const u32 kVirtualRAMSize;

#define MAX_SCRIPTS     256

#define SPRITEMEM_PTR image.spritemem + image.basesprite

#define SPRITE_VAR(x) (x ? (sSprite *)(SPRITEMEM_PTR + x) : NULL)

#define ELEMIDX(x) ((((x - 0x78) / 0x30) * 0x28) + 0x8078) // return comparable number to what we see in ST debugger

#define ENTSCR(x) alis.live_scripts[x / sizeof(sScriptLoc)]

#define ALIS_SCR_WCX    0
#define ALIS_SCR_WCY    (alis.platform.version >= 30 ? 8 : 2)
#define ALIS_SCR_WCZ    (alis.platform.version >= 30 ? 16 : 4)

#define ALIS_SCR_WCAX    (alis.platform.version >= 30 ? 24 : -1)
#define ALIS_SCR_WCAY    (alis.platform.version >= 30 ? 32 : -1)
#define ALIS_SCR_WCAZ    (alis.platform.version >= 30 ? 40 : -1)

#define ALIS_SCR_ADDR    (alis.platform.version >= 30 ? 0x32 : 0x8)

#define ALIS_SCR_WCX2    (alis.platform.version >= 30 ? 0x34 : 0x9)
#define ALIS_SCR_WCY2    (alis.platform.version >= 30 ? 0x38 : 0xa)
#define ALIS_SCR_WCZ2    (alis.platform.version >= 30 ? 0x3c : 0xb)

// =============================================================================
// MARK: - ERROR CODES
// =============================================================================
#define ALIS_ERR_FOPEN          (0x01)
#define ALIS_ERR_FWRITE         (0x07)
#define ALIS_ERR_FCREATE        (0x08)
#define ALIS_ERR_FDELETE        (0x09)
#define ALIS_ERR_CDEFSC         (0x0a)
#define ALIS_ERR_VRAM_OVERFLOW  (0x0c)
#define ALIS_ERR_FREAD          (0x0d)
#define ALIS_ERR_FCLOSE         (0x0e)
#define ALIS_ERR_FSEEK          (0x00)


typedef struct {
    u8      errnum;
    char    name[kNameMaxLen];
    char    descfmt[kDescMaxLen];
} sAlisError;


// =============================================================================
// MARK: - OPCODES
// =============================================================================
typedef void (*alisOpcode)(void);

#define DECL_OPCODE(n, f, d)    { n, f, #f, d }

typedef struct {
    u8          code;
    alisOpcode  fptr;
    char        name[kNameMaxLen];
    char        desc[kDescMaxLen];
} sAlisOpcode;

PACK_PUSH typedef struct PACK_ATTR {
    u32         vram_offset;
    u16         offset;
} sScriptLoc; PACK_POP

// =============================================================================
// MARK: - VM
// =============================================================================
// vm specs, loaded from packed main script header
typedef struct {
    // read values from packed main script
    u16     script_data_tab_len;
    u16     script_vram_tab_len;
    u32     unused;
    u32     max_allocatable_vram;
    u32     vram_to_data_offset;
    
    // computed values
    u32     script_vram_max_addr;
} sAlisSpecs;

typedef struct {
    
    u32 address;
    u32 length;
    
} sRawBlock;

typedef struct {
    
    u16 val0;
    u16 val1;
    u32 val2;
    u32 val3;
    u32 val4;
    
} sMainHeader;

typedef enum {
    
    eAlisStateStopped       = 0,
    eAlisStateRunning       = 1,
    eAlisStateSave          = 2,
    eAlisStateLoad          = 3,
} eAlisState;

typedef struct {
    // platform
    sPlatform       platform;

    sMainHeader     header;

    sAlisSpecs      specs;
    
    
    // Absolute address of vm's virtual ram.
    // On atari the operating system gives us $22400.
    #define ALIS_VM_RAM_ORG (0x22400)
    u8 *            vram_org;
    
    // On atari, it's a stack of absolute script data adresses,
    //   each address being 4 bytes long.
    // The maximum count of script data adresses is given by
    //   the packed main script header (word at offset $6).
    // This table is located at ALIS_VM_RAM_ORG.
    u32 *           script_data_orgs;
    
    // A stack of tuples made of:
    //   absolute script vram adresses (u32)
    //   offset (u16)
    //
    // Located at VRAM_ORG + (max_script_addrs * sizeof(u32))
    // On atari it's ($22400 + ($3c * 4)) ==> $224f0
    // $224f0
    sScriptLoc *    atent_ptr;
    u32 *           atprog_ptr;

//    u8              nmode;
    u8              automode;
            
    u8              fallent;
    u8              fseq;
    u8              fmuldes;
    u8              fadddes;
    u8              ferase;
            
    u32             atprog;     // 0x22400
    u32             debprog;    // 0x2edd8
    u32             finprog;
    u32             dernprog;
    u16             maxprog;
    u16             nbprog;
    
    u16             saversp;
    
    u16             fview;
    u32             valnorme;
    s16             valchamp;
    s16 *           ptrent;
    s16             tablent[128];
    s16             matent[128];

    s32             atent;      // 0x224f0
    s32             debent;     // 0x2261c
    s32             finent;
    s32             maxent;
    s16             nbent;
    s16             dernent;
            
    u32             finmem;     // 0xf6e98
            
    s32             basemem;    // 0x22400
    s32             basevar;    // 0x0
    s32             basemain;   // 0x22690

    // mouse
    u8              mousflag;
    u8              fmouse;
    u8              fremouse;
    u8              fremouse2;
    u8              butmouse;
    u32             cbutmouse;
    u16             oldmouse;
    u8 *            desmouse;
    
    s16             prevkey;

    s16             theconfig;
    
    s16             wcx;
    s16             wcy;
    s16             wcz;

    s16             wforme;
    s16             matmask;
    
    s16             poldy;
    s16             poldx;

    // true if disasm only
    u8              disasm;
    
    // true if vm is running
    eAlisState      state;
    
    // virtual 16-bit accumulator (A4)
    s16 *           acc;
    s16 *           acc_org;
    
    // MEMORY
    u8 *            mem; // host: system memory (hardware)
    
    u8              flagmain;
    
    // in atari, located at $22400
    // contains the addresses of the loaded scripts' data
    // 60 dwords max (from $22400 -> $224f0)
    // u32             script_data_offsets[MAX_SCRIPTS];
    // u8              script_id_stack[MAX_SCRIPTS]; // TODO: use a real stack ?
    // u8              script_count;
    u8              script_index;
    
    // SCRIPTS
    // global table containing all unpacked scripts
    sAlisScriptLive * live_scripts[MAX_SCRIPTS];
    sAlisScriptData * loaded_scripts[MAX_SCRIPTS];

    // pointer to current script
    sAlisScriptLive * script;
    sAlisScriptLive * main;
        
    // virtual registers
    s16             varD6;
    s16             varD7;
    
    // running script id
    u16             varD5;
    
    // string buffers
    char *          bsd7;
    char *          bsd6;
    char *          bsd7bis;
    
    char *          sd7;
    char *          sd6;
    char *          oldsd7;
    
    u32             vstandard;
    
    char            autoname[256];

    u32             tabptr;

    // data buffers
    u8              buffer[1024];
    sRawBlock       blocks[1024];
    
    u8              charmode;
    u8              xlocate;
    u8              ylocate;
    u8              pays;

    // font
    u16             foasc;
    u16             fonum;
    u8              folarg;
    u8              fohaut;
    u16             fomax;

    u32             cdspeed;

    u8              witmov;
    u8              fmitmov;
    u16             goodmat;
    u32             baseform;
    
    s8              typepack;
    u8              wordpack;
    u32             longpack;
    
    // helper: executed instructions count
    u32             icount;
    u8              restart_loop;
    
    // virtual status register
    struct {
        u8 zero: 1;
        u8 neg: 1;
    } sr;
    
    // helpers
    u8              oeval_loop;
    
    // system helpers
    FILE *          fp;
    u16             openmode;
    
    u16             vquality;
    
    // misc
    u8              fswitch;
    u8              ctiming;
    u8              cstopret;
    u16             random_number;
    
    s8              vprotect;
    
    u32             timeclock;
    
    u16             basedark;
    u32             ptrdark;
    
    struct timeval  frametime;
    struct timeval  looptime;

    u8              swap_endianness;
    
    s32             load_delay;
    s32             unload_delay;

} sAlisVM;

typedef struct {

    // system stuff
    // mouse_t     mouse;
    pixelbuf_t  pixelbuf;
} sHost;

extern sAlisVM alis;
extern sHost host;


// =============================================================================
// MARK: - API
// =============================================================================

u8              alis_init(sPlatform platform);
int             alis_thread(void *data);
void            alis_deinit(void);

void            alis_save_state(void);
void            alis_load_state(void);

void            alis_start_script(sAlisScriptData * script);
void            alis_error(int errnum, ...);
void            alis_debug_ram(void);
void            alis_debug_addr(u16 addr);

void            vram_init(void);
u8 *            get_vram(s16 offset);

int             adresdes(s32 idx);
int             adresmus(s32 idx);
int             adresform(s16 idx);

s32             tabint(u32 address);
s32             tabchar(u32 address);
s32             tabstring(u32 address);

FILE            *afopen(char *path, u16 openmode);

void            sleep_until(struct timeval *start, s32 len);
void            sleep_interactive(s32 *loop, s32 intr);
