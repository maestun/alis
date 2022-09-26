//
//  alis_vm.h
//  lc3vm
//

#ifndef __ALIS_H__
#define __ALIS_H__

#include "config.h"
#include "debug.h"
#include "platform.h"
#include "script.h"
#include "sys/sys.h"
#include "vram.h"


// #define kVMHeaderLen            (16 * sizeof(u8))
// #define kHostRAMSize            (1024 * 1024 * sizeof(u8))
// #define kVirtualRAMSize         (0xffff * sizeof(u8))
#define kMaxScripts             (256)
#define kBSSChunkLen            (256)


// =============================================================================
// MARK: - ERROR CODES
// =============================================================================
#define ALIS_ERR_FOPEN          (0x01)
#define ALIS_ERR_ENT_OVERFLOW   (0x05)
#define ALIS_ERR_FWRITE         (0x07)
#define ALIS_ERR_FCREATE        (0x08)
#define ALIS_ERR_FDELETE        (0x09)
#define ALIS_ERR_CDEFSC         (0x0a)
#define ALIS_ERR_PROG_OVERFLOW  (0x0b)
#define ALIS_ERR_VRAM_OVERFLOW  (0x0c)
#define ALIS_ERR_FREAD          (0x0d)
#define ALIS_ERR_FCLOSE         (0x0e)
#define ALIS_ERR_MAXPROG        (0x13)
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
} sScriptLoc;


// =============================================================================
// MARK: - VM
// =============================================================================

// vm specs: 16 bytes read from packed main script header
typedef struct {
    u16 max_prog;
    u16 max_ent;
    u32 max_host_ram;
    u32 debsprit_offset;
    u32 finsprit_offset;
} sAlisSpecs;


typedef struct {
    u32 addr;
    u16 word;
} sEnt;


// all these are offsets !
// for ishar 1 data w/ ishar2 interpreter:
typedef struct {

    u32 basemem;       // $22400, set once by host system's 'malloc'
    u32 basevar;       // $0
    
    // address of a dword table that holds the adresses of loaded script data
    // (max size is read in packed main script's header, at offset 0x6)
    u32 atprog;        // $22400, set once, same as basemem
    
    // address of the next available atprog slot
    u32 dernprog;      // $22400, will increment by 4 each time a script is loaded
    
    u32 atent;          // $224f0, set once: atent = basemem + (vm.specs.maxprog * sizeof(ptr_t))
                        // atent ent contains 6*MAXENT (word read in MAIN.AO header at byte 0x8)
    u32 debent;         // $2261c, set once: debent = atent + (vm.specs.maxent * 6)

    u32 basemain;       // $22690, set once: basemain = debent + vmspecs[0x12] + vmspecs[0x16] + sizeof(struct sVMStat)

    u32 finent;        // $26902

    u32 debsprit;       // $283e0, set once: debsprit = ((atent + vm.specs.debsprit_offset) & 0x0f) + 1
    u32 finsprit;       // $2d278, set once: finsprit = debsprit + ((vm.specs.finsprit_offset + 3) * 40)
    u32 basesprit;      // $31f40, set at init_sprites()
    u16 tvsprit;        // 0x8000, set at init_sprites()
    u16 backsprit;      // 0x8028, set at init_sprites()
    u16 texsprit;       // 0x8050, set at init_sprites()

    u32 debprog;       // $2d278, set once: debprog = finsprit
    u32 finprog;       // $

    u32 finmem;         // end of host memory

} sAlisMemory;





//typedef struct {
//    u8  b_mousflag;
//    u16 libsprit;
//} sAlisVars;



typedef struct {
    // platform
    sPlatform       platform;
    
    // vm specs (TODO: used to compute vram offsets, can be discarded)
    sAlisSpecs      specs;
    
    // vm memory map
    sAlisMemory     vram;
    
    // vm status (data stored before basemain / $22690)
    sVMStatus       vstat;
    
    // variables
    // sAlisVars       vars;
    
    // Absolute address of vm's virtual ram.
    // On atari the operating system gives us $22400.
    #define ALIS_VM_RAM_ORG (0x22400)
    u8 *            vram_org; // basemem
    
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
    sScriptLoc *    script_vram_orgs; // atent
    
    // true if disasm only
    u8              disasm;
    
    // true if vm is running
    u8              running;
    
    // virtual 16-bit accumulator (A4)
    u16             vacc_offset;
    s16 *           acc;
    s16 *           acc_org;
    
    // MEMORY
    u8 *            mem; // host: system memory (hardware)
    
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
    
    // pointer to current script
    sAlisScript *       script;
    // sAlisScript *       main;
        
    // virtual registers
    s16             varD6;
    s16             varD7;
    
    // branching register
    u16             varD5;
    
    // virtual accumulator offset
    u16             varD4;
    
    // virtual array registers
    u8 *           sd7;
    u8 *           sd6;
    u8 *           oldsd7;
    
    // helper: executed instructions count
    u32            icount;
            
    // virtual status register
    struct {
        u8 zero: 1;
        u8 neg: 1;
    } sr;
    
    // helpers
    u8          oeval_loop;
    
    // system helpers
    FILE *      fp;
    
    // unknown variables
    u8          _cstopret;
    u8          _callentity;
    u8          _cclipping;
    u8          _ctiming;
//    s16         _a6_minus_1a; // used by cforme
//    u16         _a6_minus_16;
    u16         _random_number;
//    u8          _xinvon; // (-0x3,A6)
    
    u16         _DAT_000195fa;
    u16         _DAT_000195fc;
    u16         _DAT_000195fe;
    
    
    // global variables declared in DATA section
    struct {
        
        // ent
        struct {
            u8 fallent;
        } ent;
        
        // font stuff
        struct {
            u16 w_foasc;
            u16 w_fonum;
            u16 w_folarg;
            u16 w_fohaut;
            u16 w_fomax;
        } font;
        
        // paper (console) stuff
        struct {
            u8 b_papercolor;
            u8 b_inkcolor;
            u8 b_pmode;
        } paper;
        
        // unknown stuff
        u16 w_poldx;
        u16 w_poldy;
        
        u8 fseq;
        
        u16 save_rsp;
        
        
        u8  b_mousflag;
        u16 libsprit;

    } vars;
    
    
} sAlisVM;

typedef struct {

    // system stuff
    mouse_t     mouse;
    pixelbuf_t  pixelbuf;
} sHost;

extern sAlisVM vm;
extern sHost host;


// =============================================================================
// MARK: - API
// =============================================================================
void            alis_init(sPlatform platform);
u8              alis_main(void);
void            alis_engine(void);
void            alis_deinit(void);
void            alis_start_script(sAlisScript * script);
void            alis_register_script(sAlisScript * script);
void            alis_error(u8 errnum, ...);
void            alis_debug(void);
void            alis_debug_ram(void);
void            alis_debug_addr(u16 addr);

#endif /* __ALIS_H__ */
