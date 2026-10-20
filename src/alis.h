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
#include "mem.h"
#include "platform.h"
#include "script.h"
#include "sys/sys.h"
#include "utils.h"

#include <setjmp.h>

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


extern const u32 kVirtualRAMSize;

#define MAX_SCRIPTS     512

#define SPRITEMEM_PTR image.spritemem + image.basesprite

// u16: indices >= 0x8000 held in s16 must not go negative
#define SPRITE_VAR(x) ((x) ? (sSprite *)(SPRITEMEM_PTR + (u16)(x)) : NULL)

#define ELEMIDX(x) ((((x - 0x78) / 0x30) * 0x28) + 0x8078) // return comparable number to what we see in ST debugger

// entity id = byte offset into atent; v6 uses 0x18-byte memory nodes (DOS io_malloc table)
#define ENT_STRIDE (alis.platform.version >= 60 ? 0x18 : (int)sizeof(sScriptLoc))
#define ENTSCR(x) alis.live_scripts[(x) / ENT_STRIDE]

#define ALIS_SCR_WCX    0
#define ALIS_SCR_WCY    (alis.platform.version >= 30 ? 8 : 2)
#define ALIS_SCR_WCZ    (alis.platform.version >= 30 ? 16 : 4)

#define ALIS_SCR_WCAX    (alis.platform.version >= 30 ? 0x18 : -1)
#define ALIS_SCR_WCAY    (alis.platform.version >= 30 ? 0x20 : -1)
#define ALIS_SCR_WCAZ    (alis.platform.version >= 30 ? 0x28 : -1)

#define ALIS_SCR_ADDR    (alis.platform.version >= 30 ? 0x32 : 0x8)
#define ALIS_SCR_GROUP   (alis.platform.version >= 30 ? 0x30 : 0x6)

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

// NDEBUG strips the per-opcode name+desc strings (only readexec's debug disassembler reads them).
#ifdef NDEBUG
# define DECL_OPCODE(n, f, d)   { .fptr = (f), .code = (n) }
#else
# define DECL_OPCODE(n, f, d)   { .fptr = (f), .code = (n), .name = #f, .desc = (d) }
#endif

// NDEBUG struct padded to 8 bytes: power-of-2 dispatch stride (scaled addressing, no muls).
typedef struct {
    alisOpcode  fptr;
    u8          code;
#ifndef NDEBUG
    char        name[kNameMaxLen];
    char        desc[kDescMaxLen];
#else
    u8          _pad[3];    // -> sizeof == 8 (power-of-2 dispatch stride)
#endif
} sAlisOpcode;

// -----------------------------------------------------------------------------
// VM hot-state register pinning: in ALIS_VM_CORE units on m68k, a4 = alis.mem and a5 = alis.script
// (pointers only, so nothing drifts; alis.mem changes need VM_SYNC_MEM, script changes go through
// ALIS_SET_SCRIPT). -DALIS_VM_REGVARS=0 falls back to plain struct access.
// -----------------------------------------------------------------------------
#if !defined(ALIS_VM_REGVARS)
# if defined(__m68k__) && defined(NDEBUG)
#  define ALIS_VM_REGVARS 1
# else
#  define ALIS_VM_REGVARS 0
# endif
#endif

#if ALIS_VM_REGVARS && defined(ALIS_VM_CORE) && defined(__m68k__)
register u8 *              vm_mem    __asm__ ("a4");
register sAlisScriptLive * vm_script __asm__ ("a5");
# define VMEM               vm_mem
# define VSCRIPT            vm_script
# define VM_SYNC_MEM()      (vm_mem = alis.mem)
# define VM_SYNC_SCRIPT()   (vm_script = alis.script)
#else
# define VMEM               (alis.mem)
# define VSCRIPT            (alis.script)
# define VM_SYNC_MEM()      ((void)0)
# define VM_SYNC_SCRIPT()   ((void)0)
#endif

// All writes to the current-script pointer must go through this so the pinned
// register (if any) stays in sync with the struct member.
#define ALIS_SET_SCRIPT(x)  do { alis.script = (x); VM_SYNC_SCRIPT(); } while(0)

// Version-specific dispatch tables, set by alis_init (v6 overrides the pre-v6 ones).
extern const sAlisOpcode *tab_opcodes, *tab_codesc1, *tab_opernames, *tab_storenames, *tab_addnames;
void readexec_opcode_v6(void);

// Fast inline dispatch macros for NDEBUG builds.
// Avoids function call overhead and 1KB struct copy per opcode.
#ifdef NDEBUG
# define READEXEC_FAST(table)       do { (table)[*(VMEM + VSCRIPT->pc++)].fptr(); } while(0)
# define readexec_opcode()          do { if (alis.platform.version >= 60) readexec_opcode_v6(); else READEXEC_FAST(opcodes); } while(0)
# define readexec_codesc1name()     READEXEC_FAST(tab_codesc1)
# define readexec_codesc2name()     READEXEC_FAST(codesc2names)
# define readexec_codesc3name()     READEXEC_FAST(codesc3names)
# define readexec_opername()        do { READEXEC_FAST(tab_opernames); if (alis.platform.version < 60) alis.varD7 = (s16)alis.varD7; } while(0)
# define readexec_storename()       READEXEC_FAST(tab_storenames)
# define readexec_addname()         READEXEC_FAST(tab_addnames)

# define readexec_addname_swap() do { \
        char *_tmp = alis.sd7; alis.sd7 = alis.oldsd7; alis.oldsd7 = _tmp; \
        readexec_addname(); \
    } while(0)

# define readexec_opername_saveD7() do { \
        alis.varD6 = alis.varD7; \
        readexec_opername(); \
    } while(0)

# define readexec_opername_saveD6() do { \
        s16 _tmp = alis.varD7; \
        readexec_opername_saveD7(); \
        alis.varD6 = alis.varD7; \
        alis.varD7 = _tmp; \
    } while(0)

# define readexec_opername_swap() do { \
        char *_tmp = alis.sd7; alis.sd7 = alis.sd6; alis.sd6 = _tmp; \
        readexec_opername(); \
    } while(0)

#endif

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
    u32     mem_cap;        // arena cap past debprog (4,000,000 in every known game)
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
    s16             tablent[256];   // NOTE: Deus can exceed 128 entities
    s16             matent[256];

    s32             atent;      // 0x224f0
    s32             debent;     // 0x2261c
    s32             finent;
    s32             maxent;
    s16             nbent;
    s16             dernent;
    u16             nodefree;   // v6 node free list head / tail (DOS fac / fb0)
    u16             nodetail;
    u16             filmnode;   // DOS film buffer node (DAT_0005403c +0x50)
    u8              videomode;  // DOS DAT_00053ef3: 0 VGA, 1 SVGA
    struct {
        u16 scr;
        u16 field;
        u16 node;
        u32 handle;
    } scrnode[16];  // screen +0xb4/+0xbc buffers
            
    u32             finmem;     // 0xf6e98
    u16             memclass;   // omodel memory class for a reduced arena (0 = legacy value)
            
    s32             basemem;    // 0x22400
    s32             basevar;    // 0x0
    s32             basemain;   // 0x22690

    // mouse
    u8              mousflag;
    u8              fmouse;
    u8              fremouse;
    u8              fremouse2;
    u16             oldmouse;
    u8 *            desmouse;
    
    s16             prevkey;

    s16             theconfig;
    s32             v6ips;
    s32             v6soundseq;
    s32             v6focstep;
    s32             v6tstdx;
    s32             v6tstdy;
    s32             v6tstdz;
    u16             v6tstmask;
    u8              v6tstflag;
    s32             v6tstarg;
    u8              v6tstpolar;
    u8              v6walkmode;
    s32             v6walkc4;
    s32             v6walkcc;
    s32             v6walkd0;
    u8              v6walkd8;
    s32             v6walkb8;
    jmp_buf         ofin_jmp;
    int             ofin_active;
    
    s16             wcx;
    s16             wcy;
    s16             wcz;

    s16             wforme;
    s16             matmask;
    
    s16             poldy;
    s16             poldx;
    
    // true if vm is running
    eAlisState      state;
    
    // virtual 16-bit (v6 32-bit) accumulator (A4)
    s32 *           acc;
    s32 *           acc_org;
    
    // MEMORY
    u8 *            mem; // host: system memory (hardware)
    
    u8              flagmain;
    
    // requiem
    
    s16 *pretrlinetra;
    s16 *pretglinetra;
    
    s16 *tlinetra;
    s16 *itlinetra;
    s16 *trlinetra;
    s16 *tglinetra;

    
    u8              script_index;
    
    // SCRIPTS
    // global table containing all unpacked scripts
    sAlisScriptLive * live_scripts[MAX_SCRIPTS];
    sAlisScriptData * loaded_scripts[MAX_SCRIPTS];

    // pointer to current script
    sAlisScriptLive * script;
    sAlisScriptLive * main;
        
    // virtual registers
    s32             varD6;
    s32             varD7;
    
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

    struct {
        s16 foasc;
        s16 fonum;
        s16 fomax;
        u8 folarg;
        u8 fohaut;
        u8 mode;
        u8 valid;
        u8 usemain;
    } v6fonts[64];
    
    struct {
        s32 id;
        u32 glyphslot;
        s32 foasc;
        s32 count;
        s32 spacing;
        s32 height;
        u8 valid;
        u8 op2;             // +0x04
        u16 owner;          // +0x1c owner entity id
        s16 form;           // +0x1e
        u8 ownername[8];    // +0x20 owner script name
    } v6xfonts[64];

    u8              fomain;
    s8              talign;
    u8              v6fontnum;
    u8              v6x;
    u32             v6tabcell;
    u32             texmap;         // 3DO etexmap slot table (0x3a254)
    u16             texcur;         // 3DO current texture id (0x3a384)
    u16             fog3do;         // 3DO cscdark fog level (screen +0x136, 0x34d68)
    u16             v6kforgx;
    u16             v6kforgy;
    u16             v6kforgz;
    u8              ftextfield;
    s16             textfx;
    s16             textfy;

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

    u8              swap_endianness;
    
    s32             load_delay;
    s32             unload_delay;

    u8 *            text_data;
    u32             text_sz;
    char            text_name[16];
    struct { u16 node; u32 sz; u8 *data; } v6texts[8];  // resident v6 .to blocks (type 0xc)

} sAlisVM;

typedef struct {

    // system stuff
    pixelbuf_t  pixelbuf;
} sHost;

extern sAlisVM alis;

// PC CGA art: 4-colour sprites and the fixed CGA palette (not the 16-colour re-release)
#define PL_CGA()        (alis.platform.kind == EPlatformPC && alis.platform.version <= 11 && !alis.platform.ega)

// program table at atprog: data_org of each loaded program, game byte order
#define ATPROG(i)       ((u32)xread32(alis.atprog + (u32)(i) * 4))
#define ATPROG_SET(i, v) xwrite32(alis.atprog + (u32)(i) * 4, (v))
extern u32 alis_arena_size;
extern const char *alis_fatal; // set to stop the VM with a message shown after shutdown
    // set by the native memory pre-flight; 0 = preferred tier
extern sHost host;

// Inlined script-bytecode accessors (release builds; debug builds use the logging versions in
// script.c). Must follow the `alis` declaration: VMEM/VSCRIPT fall back to alis.mem/alis.script
// outside VM-core TUs.
#ifdef NDEBUG
static inline u8  script_read8(void)  { return VMEM[VSCRIPT->pc++]; }
static inline u16 script_read16(void) { u16 v = read16(VMEM + VSCRIPT->pc); VSCRIPT->pc += 2; return v; }
static inline u32 script_read24(void) { u32 v = read24(VMEM + VSCRIPT->pc); VSCRIPT->pc += 3; return v; }
static inline u32 script_read32(void) { u32 v = read32(VMEM + VSCRIPT->pc); VSCRIPT->pc += 4; return v; }
#endif


// =============================================================================
// MARK: - Script VRAM / screen-element accessors (inline)
// =============================================================================

// Script VRAM accessors — indexed from a script's vram origin.
static inline u32 get_0x3e_wait_time(u32 vram)                        { return xread32(vram - 0x3e); }
static inline u16 get_0x3a_wait_cycles(u32 vram)                      { return xread16(vram - 0x3a); }
static inline u16 get_0x38_unknown(u32 vram)                          { return xread16(vram - 0x38); }
static inline u16 get_0x36_unknown(u32 vram)                          { return xread16(vram - 0x36); }
static inline u16 get_0x34_unknown(u32 vram)                          { return xread16(vram - 0x34); }
static inline u8 get_0x32_unknown(u32 vram)                           { return xread8(vram - 0x32); }
static inline u8 get_0x31_unknown(u32 vram)                           { return xread8(vram - 0x31); }
static inline u8 get_0x30_unknown(u32 vram)                           { return xread8(vram - 0x30); }
static inline u8 get_0x2f_chsprite(u32 vram)                          { return xread8(vram - 0x2f); }
static inline u8 get_0x2e_script_header_word_2(u32 vram)              { return xread8(vram - 0x2e); }
static inline u8 get_0x2d_calign(u32 vram)                            { return xread8(vram - 0x2d); }
static inline u8 get_0x2c_calign(u32 vram)                            { return xread8(vram - 0x2c); }
static inline u8 get_0x2b_cordspr(u32 vram)                           { return xread8(vram - 0x2b); }
static inline u16 get_0x2a_clinking(u32 vram)                         { return xread16(vram - 0x2a); }
static inline u8 get_0x28_unknown(u32 vram)                           { return xread8(vram - 0x28); }
static inline u8 get_0x27_creducing(u32 vram)                         { return xread8(vram - 0x27); }
static inline u8 get_0x26_creducing(u32 vram)                         { return xread8(vram - 0x26); }
static inline u8 get_0x25_credon_credoff(u32 vram)                    { return xread8(vram - 0x25); }
static inline s8 get_0x24_scan_inter(u32 vram)                        { return xread8(vram - 0x24); }
static inline u8 get_0x23_unknown(u32 vram)                           { return xread8(vram - 0x23); }
static inline u16 get_0x22_cworld(u32 vram)                           { return xread16(vram - 0x22); }
static inline u16 get_0x20_set_vect(u32 vram)                         { return xread16(vram - 0x20); }
static inline s16 get_0x1e_scan_clr(u32 vram)                         { return xread16(vram - 0x1e); }
static inline s16 get_0x1c_scan_clr(u32 vram)                         { return xread16(vram - 0x1c); }
static inline s16 get_0x1a_cforme(u32 vram)                           { return xread16(vram - 0x1a); }
static inline u16 get_0x18_unknown(u32 vram)                          { return xread16(vram - 0x18); }
static inline u16 get_0x16_screen_id(u32 vram)                        { return xread16(vram - 0x16); }
static inline u32 get_0x14_script_org_offset(u32 vram)                { return xread32(vram - 0x14); }
static inline u16 get_0x10_script_id(u32 vram)                        { return xread16(vram - 0x10); }
static inline u16 get_0x0e_script_ent(u32 vram)                       { return xread16(vram - 0xe); }
static inline s16 get_0x0c_vacc_offset(u32 vram)                      { return xread16(vram - 0xc); }
// v6 casleepfar "far asleep" flag (DOS ctx [-0x1] bit 1, 3DO [r6-1] bit 1): set by casleepfar
// when the entity is beyond the radius, cleared (with a wake message) when it comes back in.
// The DOS scheduler skips such an entity entirely (moteur1 0000538a). Our ctx byte at -0x65 is
// otherwise unused (see the v6 context map in script.c get_context_size).
#define V6_FARSLEEP(vram) ((vram) - 0x65)
#define V6_DORMANT 8      // same byte, DOS ctx -1 bit 3 (cunload mode 2)
// v6 clinking register (DOS / 3DO ctx -0x14): own id at live, clinking 0x83 sets it, put copies it to slot +0x4e
#define V6_CLINKING(vram) ((vram) - 0x7a)
static inline s16 get_0x0a_vacc_offset(u32 vram)                      { return xread16(vram - 0xa); }
static inline u32 get_0x08_script_ret_offset(u32 vram)                { return xread32(vram - 0x8); }
static inline u8 get_0x04_cstart_csleep(u32 vram)                     { return xread8(vram - 0x4); }
static inline u8 get_0x03_xinv(u32 vram)                              { return xread8(vram - 0x3); }
static inline u8 get_0x02_wait_cycles(u32 vram)                       { return xread8(vram - 0x2); }
static inline u8 get_0x01_wait_count(u32 vram)                        { return xread8(vram - 0x1); }

static inline void set_0x3e_wait_time(u32 vram, u32 val)              { xwrite32(vram - 0x3e, val); }
static inline void set_0x3a_wait_cycles(u32 vram, u16 val)            { xwrite16(vram - 0x3a, val); }
static inline void set_0x38_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x38, val); }
static inline void set_0x36_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x36, val); }
static inline void set_0x34_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x34, val); }
static inline void set_0x32_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x32, val); }
static inline void set_0x31_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x31, val); }
static inline void set_0x30_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x30, val); }
static inline void set_0x2f_chsprite(u32 vram, u8 val)                { xwrite8(vram - 0x2f, val); }
static inline void set_0x2e_script_header_word_2(u32 vram, u8 val)    { xwrite8(vram - 0x2e, val); }
static inline void set_0x2d_calign(u32 vram, u8 val)                  { xwrite8(vram - 0x2d, val); }
static inline void set_0x2c_calign(u32 vram, u8 val)                  { xwrite8(vram - 0x2c, val); }
static inline void set_0x2b_cordspr(u32 vram, u8 val)                 { xwrite8(vram - 0x2b, val); }
static inline void set_0x2a_clinking(u32 vram, u16 val)               { xwrite16(vram - 0x2a, val); }
static inline void set_0x28_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x28, val); }
static inline void set_0x27_creducing(u32 vram, u8 val)               { xwrite8(vram - 0x27, val); }
static inline void set_0x26_creducing(u32 vram, u8 val)               { xwrite8(vram - 0x26, val); }
static inline void set_0x25_credon_credoff(u32 vram, u8 val)          { xwrite8(vram - 0x25, val); }
static inline void set_0x24_scan_inter(u32 vram, s8 val)              { xwrite8(vram - 0x24, val); }
static inline void set_0x23_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x23, val); }
static inline void set_0x22_cworld(u32 vram, u16 val)                 { xwrite16(vram - 0x22, val); }
static inline void set_0x20_set_vect(u32 vram, u16 val)               { xwrite16(vram - 0x20, val); }
static inline void set_0x1e_scan_clr(u32 vram, s16 val)               { xwrite16(vram - 0x1e, val); }
static inline void set_0x1c_scan_clr(u32 vram, s16 val)               { xwrite16(vram - 0x1c, val); }
static inline void set_0x1a_cforme(u32 vram, s16 val)                 { xwrite16(vram - 0x1a, val); }
static inline void set_0x18_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x18, val); }
static inline void set_0x16_screen_id(u32 vram, u16 val)              { xwrite16(vram - 0x16, val); }
static inline void set_0x14_script_org_offset(u32 vram, u32 val)      { xwrite32(vram - 0x14, val); }
static inline void set_0x10_script_id(u32 vram, u16 val)              { xwrite16(vram - 0x10, val); }
static inline void set_0x0e_script_ent(u32 vram, u16 val)             { xwrite16(vram - 0x0e, val); }
static inline void set_0x0c_vacc_offset(u32 vram, s16 val)            { xwrite16(vram - 0x0c, val); }
static inline void set_0x0a_vacc_offset(u32 vram, s16 val)            { xwrite16(vram - 0x0a, val); }
static inline void set_0x08_script_ret_offset(u32 vram, u32 val)      { xwrite32(vram - 0x08, val); }
static inline void set_0x04_cstart_csleep(u32 vram, u8 val)           { xwrite8(vram - 0x04, val); }
static inline void set_0x03_xinv(u32 vram, u8 val)                    { xwrite8(vram - 0x03, val); }
static inline void set_0x02_wait_cycles(u32 vram, u8 val)             { xwrite8(vram - 0x02, val); }
static inline void set_0x01_wait_count(u32 vram, u8 val)              { xwrite8(vram - 0x01, val); }

// =============================================================================
// MARK: - API
// =============================================================================

u8              alis_init(sPlatform platform);
int             alis_thread(void *data);
void            alis_deinit(void);

u16             alis_first_ent(void);

void            alis_save_state(void);
void            alis_load_state(void);

void            alis_start_script(sAlisScriptData * script);
void            alis_error(int errnum, ...);
void            alis_debug_ram(void);
void            alis_debug_addr(u16 addr);

void            vram_init(void);

int             adresdes(s32 idx);
int             adresmus(s32 idx);
int             adresform(s16 idx);

s32             tabint(u32 address);
s32             v6_tabcell(u32 address, int shift);
u32             read_vaddr(void);
void            v6_scdark_precalc(u32 scr);
s32             tabchar(u32 address);
s32             tabstring(u32 address);

FILE            *afopen(char *path, u16 openmode);
