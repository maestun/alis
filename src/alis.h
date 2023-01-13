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
#pragma mark - ERROR CODES
// =============================================================================
#define ALIS_ERR_FOPEN          (0x01)
#define ALIS_ERR_ENT_OVERFLOW   (0x05)
#define ALIS_ERR_FWRITE         (0x07)
#define ALIS_ERR_FCREATE        (0x08)
#define ALIS_ERR_FDELETE        (0x09)
#define ALIS_ERR_CDEFSC         (0x0a)
#define ALIS_ERR_VRAM_OVERFLOW  (0x0b)
#define ALIS_ERR_SPRIT_OVERFLOW (0x0c)
#define ALIS_ERR_FREAD          (0x0d)
#define ALIS_ERR_FCLOSE         (0x0e)
#define ALIS_ERR_MAXPROG        (0x13)
#define ALIS_ERR_VECTOR         (0x14)
#define ALIS_ERR_MEMFEN         (0x16)
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
    // sVMContext      context;
    
    // variables
    // sAlisVars       vars;
    
    
    // u8 *            vram_org; // basemem
    
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
    
    // virtual accumulator (A4)
    // can store 64 bytes
    u16             vacc_offset;
    s16 *           acc;
    s16 *           acc_org;
    u8              arr_calc_sp[64];
    
    // MEMORY
    // u8 *            mem; // host: system memory (hardware)
    
    // in atari, located at $22400
    // contains the addresses of the loaded scripts' data
    // 60 dwords max (from $22400 -> $224f0)
    // u32             script_data_offsets[kMaxScripts];
    // u8              script_id_stack[kMaxScripts]; // TODO: use a real stack ?
    // u8              script_count;
    u8              script_index;
    
    // SCRIPTS
    // global table containing all depacked scripts
    // sAlisScript *   scripts[kMaxScripts];
    
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
        
        u16 nb_prog;
        
        
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
void            alis_error(u8 errnum, ...);
void            alis_debug(void);
void            alis_debug_ram(void);
void            alis_debug_addr(u16 addr);

#endif /* __ALIS_H__ */
