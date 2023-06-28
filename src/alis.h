//
//  alis_vm.h
//  lc3vm
//

#ifndef alis_vm_h
#define alis_vm_h

#include <sys/time.h>

#include "config.h"
#include "debug.h"
#include "platform.h"
#include "script.h"
#include "sys/sys.h"
// #include "vram.h"


// #define kVMHeaderLen            (16 * sizeof(u8))
// #define kHostRAMSize            (1024 * 1024 * sizeof(u8))
// #define kVirtualRAMSize         (0xffff * sizeof(u8))
#define kHostRAMSize            (1024 * 1024)
#define kVirtualRAMSize         (0xffff * sizeof(u8))

#define kMaxScripts             (256)
#define kBSSChunkLen            (256)

#define SPRITEMEM_PTR alis.spritemem + alis.basesprite

#define SPRITE_VAR(x) (x ? (SpriteVariables *)(SPRITEMEM_PTR + x) : NULL)

#define ELEMIDX(x) ((((x - 0x78) / 0x30) * 0x28) + 0x8078) // return comparable number to what we see in ST debugger

#define ENTSCR(x) alis.scripts[x / sizeof(sScriptLoc)]


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
typedef void    alisRet;
typedef alisRet (*alisOpcode)(void);

#define DECL_OPCODE(n, f, d)    { n, f, #f, d }

typedef struct {
    u8          code;
    alisOpcode  fptr;
    char        name[kNameMaxLen];
    char        desc[kDescMaxLen];
} sAlisOpcode;


typedef struct __attribute__((packed)) {
    u32         vram_offset;
    u16         offset;
} sScriptLoc ;

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
    // platform
    sPlatform       platform;
    
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

    u8              nmode;
    u8              automode;
    s8              numelem;
            
    u8              fallent;
    u8              fseq;
    u8              flaginvx;
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
            
    s32             basemem;   // 0x22400
    s32             basevar;   // 0x0
    s32             basemain;   // 0x22690
    s32             basesprite;
    u16             libsprit;
    s32             debsprit;   // 0x29f40
    s32             finsprit;   // 0x2edd8
    s32             backsprite;
    s32             tvsprite;
    s32             texsprite;
    s32             atexsprite;

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

    s16             depx;
    s16             depy;
    s16             depz;
    
    s16             wcx;
    s16             wcy;
    s16             wcz;
    
    s16             oldcx;
    s16             oldcy;
    s16             oldcz;

    s16             wforme;
    s16             matmask;
    
    s16             poldy;
    s16             poldx;

    // true if disasm only
    u8              disasm;
    
    // true if vm is running
    u8              running;
    
    // virtual 16-bit accumulator (A4)
    s16 *           acc;
    s16 *           acc_org;
    
    // MEMORY
    u8 *            mem; // host: system memory (hardware)
    
    u8              flagmain;
    
    // in atari, located at $22400
    // contains the addresses of the loaded scripts' data
    // 60 dwords max (from $22400 -> $224f0)
    // u32             script_data_offsets[kMaxScripts];
    // u8              script_id_stack[kMaxScripts]; // TODO: use a real stack ?
    // u8              script_count;
    u8              script_index;
    
    // SCRIPTS
    // global table containing all depacked scripts
    sAlisScript *   scripts[kMaxScripts];
    sAlisScript *   progs[kMaxScripts];

    // pointer to current script
    sAlisScript *       script;
    sAlisScript *       main;
        
    // virtual registers
    s16             varD6;
    s16             varD7;
    
    // branching register
    u16             varD5;
    
    // virtual array registers
    u8 *           bsd7;
    u8 *           bsd6;
    u8 *           bsd7bis;
    
    u8 *           sd7;
    u8 *           sd6;
    u8 *           oldsd7;

    u8             buffer[1024];
    sRawBlock      blocks[1024];
    
    u8 charmode;
    
    // font
    u16 foasc;
    u16 fonum;
    u8 folarg;
    u8 fohaut;
    u16 fomax;
    
    u8 witmov;
    u8 fmitmov;
    u16 goodmat;
    u32 baseform;
    
    // helper: executed instructions count
    u32             icount;
    u8              restart_loop;
        
    // unknown vars
    u32 DAT_000194fe;
    
    // virtual status register
    struct {
        u8 zero: 1;
        u8 neg: 1;
    } sr;
    
    // helpers
    u8          oeval_loop;
    
    // system helpers
    FILE *      fp;
    u16         openmode;
    
    // sound
    u8 volson;
    u8 typeson;
    u8 pereson;
    u8 priorson;
    u16 freqson;
    u16 longson;
    u16 dfreqson;
    u16 dvolson;
    u8 volsam;
    u8 speedsam;
    u16 loopsam;
    u32 startsam;
    u32 longsam;
    u16 freqsam;
    u16 vquality;
    
    // unknown variables
    u8          _cstopret;
    u8          _callentity;
    u8          fswitch;
    u8          _ctiming;
//    s16         _a6_minus_1a; // used by cforme
//    u16         _a6_minus_16;
    u16         _random_number;
//    u8          _xinvon; // (-0x3,A6)
    
    u8 *        spritemem;
    
    struct timeval time;

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

void            alis_init(sPlatform platform);
u8              alis_main(void);
void            alis_deinit(void);
void            alis_start_script(sAlisScript * script);
void            alis_error(int errnum, ...);
void            alis_debug(void);
void            alis_debug_ram(void);
void            alis_debug_addr(u16 addr);

void            alis_loop(void);

u8 *            get_vram(s16 offset);

u16             xswap16(u16 value);
u32             xswap24(u32 value);
u32             xswap32(u32 value);

u8              xread8(u32 offset);
s16             xread16(u32 offset);
s32             xread32(u32 offset);

void            xwrite8(u32 offset, u8 value);
void            xwrite16(u32 offset, s16 value);
void            xwrite32(u32 offset, s32 value);

void            xadd8(s32 offset, s8 value);
void            xadd16(s32 offset, s16 value);
void            xadd32(s32 offset, s32 add);

void            xsub8(s32 offset, s8 value);
void            xsub16(s32 offset, s16 value);
void            xsub32(s32 offset, s32 sub);

void            xpush32(s32 value);
s32             xpeek32(void);
s32             xpop32(void);

u8              vread8(u32 offset);
s16             vread16(u32 offset);
s32             vread32(u32 offset);

void            vwrite8(u32 offset, u8 value);
void            vwrite16(u32 offset, s16 value);
void            vwrite32(u32 offset, s32 value);

#endif /* alis_vm_h */
