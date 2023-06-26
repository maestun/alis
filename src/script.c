//
//  script.c
//  alis
//

#include "alis.h"
#include "unpack.h"
#include "debug.h"
#include "platform.h"
#include "script.h"
#include "utils.h"
#include "alis_private.h"
#include "experimental.h"


// TODO: for debugging
// adresses où sont stockés les scripts dans Steem
// attention, le header de kScriptHeaderLen bytes est inclus,
// donc le code commence à (adresse + kScriptHeaderLen)
typedef struct {
    u16 id;
    u8  name[kNameMaxLen];
    u32 data_org;           // location of script data in host ram (code starts 24 bytes after)
    u32 vram_org;           // location of script vram in host ram
    u16 vacc_off;
} sScriptDebug;
sScriptDebug script_debug_data[kMaxScripts] = {
    { 0x00, "MAIN", 0x2d278, 0x22690, 0xffac },
    { 0x01, "", 0x00, 0x00, 0xffac },
    { 0x02, "", 0x00, 0x00, 0xffac },
    { 0x03, "", 0x00, 0x00, 0xffac },
    { 0x04, "", 0x00, 0x00, 0xffac },
    { 0x05, "", 0x00, 0x00, 0xffac },
    { 0x06, "", 0x00, 0x00, 0xffac },
    { 0x07, "", 0x00, 0x00, 0xffac },
    { 0x08, "", 0x00, 0x00, 0xffac },
    { 0x09, "", 0x00, 0x00, 0xffac },
    { 0x0a, "", 0x00, 0x00, 0xffac },
    { 0x0b, "", 0x00, 0x00, 0xffac },
    { 0x0c, "", 0x00, 0x00, 0xffac },
    { 0x0d, "", 0x00, 0x00, 0xffac },
    { 0x0e, "", 0x00, 0x00, 0xffac },
    { 0x0f, "", 0x00, 0x00, 0xffac },
    
    { 0x10, "", 0x00, 0x00, 0xffac },
    { 0x11, "", 0x00, 0x00, 0xffac },
    { 0x12, "", 0x00, 0x00, 0xffac },
    { 0x13, "", 0x00, 0x00, 0xffac },
    { 0x14, "", 0x00, 0x00, 0xffac },
    { 0x15, "", 0x00, 0x00, 0xffac },
    { 0x16, "", 0x00, 0x00, 0xffac },
    { 0x17, "", 0x00, 0x00, 0xffac },
    { 0x18, "", 0x00, 0x00, 0xffac },
    { 0x19, "", 0x00, 0x00, 0xffac },
    { 0x1a, "", 0x00, 0x00, 0xffac },
    { 0x1b, "", 0x00, 0x00, 0xffac },
    { 0x1c, "", 0x00, 0x00, 0xffac },
    { 0x1d, "", 0x00, 0x00, 0xffac },
    { 0x1e, "", 0x00, 0x00, 0xffac },
    { 0x1f, "", 0x00, 0x00, 0xffac },
    
    { 0x20, "", 0x00, 0x00, 0xffac },
    { 0x21, "", 0x00, 0x00, 0xffac },
    { 0x22, "", 0x00, 0x00, 0xffac },
    { 0x23, "", 0x00, 0x00, 0xffac },
    { 0x24, "", 0x00, 0x00, 0xffac },
    { 0x25, "", 0x00, 0x00, 0xffac },
    { 0x26, "", 0x00, 0x00, 0xffac },
    { 0x27, "", 0x00, 0x00, 0xffac },
    { 0x28, "", 0x00, 0x00, 0xffac },
    { 0x29, "", 0x00, 0x00, 0xffac },
    { 0x2a, "", 0x00, 0x00, 0xffac },
    { 0x2b, "", 0x00, 0x00, 0xffac },
    { 0x2c, "", 0x00, 0x00, 0xffac },
    { 0x2d, "", 0x00, 0x00, 0xffac },
    { 0x2e, "", 0x00, 0x00, 0xffac },
    { 0x2f, "", 0x00, 0x00, 0xffac },
    
    { 0x30, "", 0x00, 0x00, 0xffac },
    { 0x31, "", 0x00, 0x00, 0xffac },
    { 0x32, "", 0x00, 0x00, 0xffac },
    { 0x33, "", 0x00, 0x00, 0xffac },
    { 0x34, "", 0x00, 0x00, 0xffac },
    { 0x35, "", 0x00, 0x00, 0xffac },
    { 0x36, "", 0x00, 0x00, 0xffac },
    { 0x37, "", 0x00, 0x00, 0xffac },
    { 0x38, "", 0x00, 0x00, 0xffac },
    { 0x39, "", 0x00, 0x00, 0xffac },
    { 0x3a, "", 0x00, 0x00, 0xffac },
    { 0x3b, "", 0x00, 0x00, 0xffac },
    { 0x3c, "", 0x00, 0x00, 0xffac },
    { 0x3d, "", 0x00, 0x00, 0xffac },
    { 0x3e, "", 0x00, 0x00, 0xffac },
    { 0x3f, "", 0x00, 0x00, 0xffac },
    
    { 0x40, "LOGO", 0x33568, 0x26976, 0xffac },
    { 0x41, "", 0x00, 0x00, 0xffac },
    { 0x42, "", 0x00, 0x00, 0xffac },
    { 0x43, "", 0x00, 0x00, 0xffac },
    { 0x44, "", 0x00, 0x00, 0xffac },
    { 0x45, "", 0x00, 0x00, 0xffac },
    { 0x46, "", 0x00, 0x00, 0xffac },
    { 0x47, "", 0x00, 0x00, 0xffac },
    { 0x48, "", 0x00, 0x00, 0xffac },
    { 0x49, "", 0x00, 0x00, 0xffac },
    { 0x4a, "", 0x00, 0x00, 0xffac },
    { 0x4b, "", 0x00, 0x00, 0xffac },
    { 0x4c, "", 0x00, 0x00, 0xffac },
    { 0x4d, "", 0x00, 0x00, 0xffac },
    { 0x4e, "", 0x00, 0x00, 0xffac },
    { 0x4f, "", 0x00, 0x00, 0xffac },
    
    { 0x50, "", 0x00, 0x00, 0xffac },
    { 0x51, "", 0x00, 0x00, 0xffac },
    { 0x52, "", 0x00, 0x00, 0xffac },
    { 0x53, "", 0x00, 0x00, 0xffac },
    { 0x54, "", 0x00, 0x00, 0xffac },
    { 0x55, "", 0x00, 0x00, 0xffac },
    { 0x56, "", 0x00, 0x00, 0xffac },
    { 0x57, "", 0x00, 0x00, 0xffac },
    { 0x58, "", 0x00, 0x00, 0xffac },
    { 0x59, "", 0x00, 0x00, 0xffac },
    { 0x5a, "", 0x00, 0x00, 0xffac },
    { 0x5b, "", 0x00, 0x00, 0xffac },
    { 0x5c, "", 0x00, 0x00, 0xffac },
    { 0x5d, "", 0x00, 0x00, 0xffac },
    { 0x5e, "", 0x00, 0x00, 0xffac },
    { 0x5f, "", 0x00, 0x00, 0xffac },
    
    { 0x60, "", 0x00, 0x00, 0xffac },
    { 0x61, "", 0x00, 0x00, 0xffac },
    { 0x62, "", 0x00, 0x00, 0xffac },
    { 0x63, "", 0x00, 0x00, 0xffac },
    { 0x64, "", 0x00, 0x00, 0xffac },
    { 0x65, "", 0x00, 0x00, 0xffac },
    { 0x66, "", 0x00, 0x00, 0xffac },
    { 0x67, "", 0x00, 0x00, 0xffac },
    { 0x68, "", 0x00, 0x00, 0xffac },
    { 0x69, "", 0x00, 0x00, 0xffac },
    { 0x6a, "", 0x00, 0x00, 0xffac },
    { 0x6b, "", 0x00, 0x00, 0xffac },
    { 0x6c, "", 0x00, 0x00, 0xffac },
    { 0x6d, "", 0x00, 0x00, 0xffac },
    { 0x6e, "", 0x00, 0x00, 0xffac },
    { 0x6f, "", 0x00, 0x00, 0xffac },
    
    { 0x70, "", 0x00, 0x00, 0xffac },
    { 0x71, "", 0x00, 0x00, 0xffac },
    { 0x72, "", 0x00, 0x00, 0xffac },
    { 0x73, "", 0x00, 0x00, 0xffac },
    { 0x74, "", 0x00, 0x00, 0xffac },
    { 0x75, "", 0x00, 0x00, 0xffac },
    { 0x76, "", 0x00, 0x00, 0xffac },
    { 0x77, "", 0x00, 0x00, 0xffac },
    { 0x78, "", 0x00, 0x00, 0xffac },
    { 0x79, "", 0x00, 0x00, 0xffac },
    { 0x7a, "", 0x00, 0x00, 0xffac },
    { 0x7b, "", 0x00, 0x00, 0xffac },
    { 0x7c, "", 0x00, 0x00, 0xffac },
    { 0x7d, "", 0x00, 0x00, 0xffac },
    { 0x7e, "", 0x00, 0x00, 0xffac },
    { 0x7f, "", 0x00, 0x00, 0xffac },
    
    { 0x80, "", 0x00, 0x00, 0xffac },
    { 0x81, "", 0x00, 0x00, 0xffac },
    { 0x82, "", 0x00, 0x00, 0xffac },
    { 0x83, "", 0x00, 0x00, 0xffac },
    { 0x84, "", 0x00, 0x00, 0xffac },
    { 0x85, "", 0x00, 0x00, 0xffac },
    { 0x86, "", 0x00, 0x00, 0xffac },
    { 0x87, "", 0x00, 0x00, 0xffac },
    { 0x88, "", 0x00, 0x00, 0xffac },
    { 0x89, "", 0x00, 0x00, 0xffac },
    { 0x8a, "", 0x00, 0x00, 0xffac },
    { 0x8b, "", 0x00, 0x00, 0xffac },
    { 0x8c, "", 0x00, 0x00, 0xffac },
    { 0x8d, "", 0x00, 0x00, 0xffac },
    { 0x8e, "", 0x00, 0x00, 0xffac },
    { 0x8f, "", 0x00, 0x00, 0xffac },
    
    { 0x90, "", 0x00, 0x00, 0xffac },
    { 0x91, "", 0x00, 0x00, 0xffac },
    { 0x92, "", 0x00, 0x00, 0xffac },
    { 0x93, "", 0x00, 0x00, 0xffac },
    { 0x94, "", 0x00, 0x00, 0xffac },
    { 0x95, "", 0x00, 0x00, 0xffac },
    { 0x96, "", 0x00, 0x00, 0xffac },
    { 0x97, "", 0x00, 0x00, 0xffac },
    { 0x98, "", 0x00, 0x00, 0xffac },
    { 0x99, "", 0x00, 0x00, 0xffac },
    { 0x9a, "", 0x00, 0x00, 0xffac },
    { 0x9b, "", 0x00, 0x00, 0xffac },
    { 0x9c, "", 0x00, 0x00, 0xffac },
    { 0x9d, "", 0x00, 0x00, 0xffac },
    { 0x9e, "", 0x00, 0x00, 0xffac },
    { 0x9f, "", 0x00, 0x00, 0xffac },
};


static uint8_t * p_depak = NULL;
static uint8_t * p_depak_end = NULL;
static uint8_t * p_pak = NULL;
static uint8_t * p_pak_end = NULL;
static uint8_t * p_dic;


// =============================================================================
// MARK: - Depacker
// =============================================================================

// byte 0 -> magic
// byte 1..3 -> depacked size (24 bits)
#define HEADER_MAGIC_SZ     (sizeof(u32))
// byte 4..5 -> main script if zero
#define HEADER_CHECK_SZ     (sizeof(u16))
// if main: byte 6..21 -> main header
#define HEADER_MAIN_SZ      (16 * sizeof(u8))
// if main: byte 22..29 -> dic
// if not main: byte 6..13 -> dic
#define HEADER_DIC_SZ       (2 * sizeof(u32))

u8 is_packedx(u32 magic) {
    return ((magic >> 24) & 0xf0) == 0xa0;
}

u32 get_depacked_size(u32 magic) {
    return (magic & 0x00ffffff);
}

int is_main(u16 check) {
    return kMainScriptID == check;
}

// =============================================================================
// MARK: - Script API
// =============================================================================

int search_insert(u32 *nums, u32 size, int target_id) {
    int start = 0;
    int end = size - 1;
        
    while(start <= end) {
        int mid = start + (end - start) / 2;
        int current_id = read16(alis.mem + nums[mid], alis.platform.is_little_endian);
        if (current_id == target_id)
            return mid;
        
        if (current_id < target_id)
            start = mid + 1;
        else
            end = mid - 1;
    }
    
    return start;
}

sAlisScript * script_init(char * name, u8 * data, u32 data_sz) {
    
    s16 id = swap16((data + 0), alis.platform.is_little_endian);
    s16 insert = debprotf(id);
    if (insert > 0 && insert < alis.nbprog)
    {
        sAlisScript *script = alis.progs[insert];
        if (script->header.id == id)
        {
            script->sz = data_sz;
            
            // script data
            script->header.id = swap16((data + 0), alis.platform.is_little_endian);
            
            if (alis.platform.kind == EPlatformPC)
            {
                // script->header.w_0x1700 = swap16((data + 4), alis.platform.is_little_endian);
                script->header.unknown01 = *(data + 4);
                script->header.unknown02 = *(data + 5);
                script->header.code_loc_offset = swap16((data + 2), alis.platform.is_little_endian);
            }
            else
            {
                // script->header.w_0x1700 = swap16((data + 2), alis.platform.is_little_endian);
                script->header.unknown01 = *(data + 2);
                script->header.unknown02 = *(data + 3);
                script->header.code_loc_offset = swap16((data + 4), alis.platform.is_little_endian);
            }
            
            script->header.ret_offset = swap32((data + 6), alis.platform.is_little_endian);
            script->header.dw_unknown3 = swap32((data + 10), alis.platform.is_little_endian);
            script->header.dw_unknown4 = swap32((data + 14), alis.platform.is_little_endian);
            script->header.w_unknown5 = swap16((data + 18), alis.platform.is_little_endian);
            script->header.vram_alloc_sz = swap16((data + 20), alis.platform.is_little_endian);
            script->header.w_unknown7 = swap16((data + 22), alis.platform.is_little_endian);
            memcpy(alis.mem + script->data_org, data, data_sz);
            
            for (int i = 0; i < alis.nbprog; i++)
            {
                debug(EDebugInfo, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.progs[i]->name, read16(alis.mem + alis.atprog_ptr[i], alis.platform.is_little_endian), alis.atprog_ptr[i]);
            }
            
            debug(EDebugInfo, "\n");

            script->pc = script->pc_org = 0;
            debug(EDebugInfo, " (10NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);

            debug(EDebugInfo, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
            return script;
        }
    }
    
    // init script
    sAlisScript * script = (sAlisScript *)malloc(sizeof(sAlisScript));
    memset(script, 0, sizeof(sAlisScript));
    strcpy(script->name, name);
    script->sz = data_sz;
    
    // script data
    script->header.id = swap16((data + 0), alis.platform.is_little_endian);
    
    if (alis.platform.kind == EPlatformPC)
    {
        script->header.unknown01 = *(data + 4);
        script->header.unknown02 = *(data + 5);
        script->header.code_loc_offset = swap16((data + 2), alis.platform.is_little_endian);
    }
    else
    {
        script->header.unknown01 = *(data + 2);
        script->header.unknown02 = *(data + 3);
        script->header.code_loc_offset = swap16((data + 4), alis.platform.is_little_endian);
    }
    
    script->header.ret_offset = swap32((data + 6), alis.platform.is_little_endian);
    script->header.dw_unknown3 = swap32((data + 10), alis.platform.is_little_endian);
    script->header.dw_unknown4 = swap32((data + 14), alis.platform.is_little_endian);
    script->header.w_unknown5 = swap16((data + 18), alis.platform.is_little_endian);
    script->header.vram_alloc_sz = swap16((data + 20), alis.platform.is_little_endian);
    script->header.w_unknown7 = swap16((data + 22), alis.platform.is_little_endian);
    
    script->vram_org = 0;
    script->vacc_off = 0;
    script->data_org = alis.finprog;

    memcpy(alis.mem + script->data_org, data, data_sz);

    // get insert point
    insert = search_insert(alis.atprog_ptr, alis.nbprog, script->header.id);

    alis.finprog += data_sz;
    alis.dernprog += 4;
    alis.nbprog ++;

    for (int i = alis.nbprog - 2; i >= insert; i--)
    {
        alis.atprog_ptr[i + 1] = alis.atprog_ptr[i];
        alis.progs[i + 1] = alis.progs[i];
    }

    alis.atprog_ptr[insert] = script->data_org;
    alis.progs[insert] = script;

    for (int i = 0; i < alis.nbprog; i++)
    {
        debug(EDebugInfo, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.progs[i]->name, read16(alis.mem + alis.atprog_ptr[i], alis.platform.is_little_endian), alis.atprog_ptr[i]);
    }
    
    debug(EDebugInfo, "\n");

    script->pc = script->pc_org = 0;
    debug(EDebugInfo, " (11NAME: %s VACC: 0x%x, PC: 0x%x) ", script->name, script->vacc_off, script->pc);

    debug(EDebugInfo, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
    
    return script;
}

void  script_live(sAlisScript * script) {
    u8 *data = alis.mem + script->data_org;

    s32 prevfin = swap16((data + 0x12), alis.platform.is_little_endian) + alis.finent;

    alis.finent = ((swap16((data + 0x16), alis.platform.is_little_endian) + (swap16((data + 0x12), alis.platform.is_little_endian) + alis.finent)) + sizeof(sScriptContext));
    
    script->vacc_off = (prevfin - alis.finent) & 0xffff;
    script->vram_org = alis.finent;
    
    u16 vram_length = swap16((data + 0x14), alis.platform.is_little_endian);
    memset(alis.mem + script->vram_org, 0, vram_length);

    s16 curent = alis.varD5;
    alis.varD7 = alis.dernent;
    
    int caller_idx = curent / sizeof(sScriptLoc);
    int script_idx = alis.dernent / sizeof(sScriptLoc);

    debug(EDebugInfo, " add at idx: %d hooked to idx: %d. ", script_idx, caller_idx);

    set_0x0a_vacc_offset(script->vram_org, script->vacc_off);
    set_0x1c_scan_clr(script->vram_org, script->vacc_off);
    set_0x1e_scan_clr(script->vram_org, script->vacc_off);
    set_0x08_script_ret_offset(script->vram_org, script->data_org + script->header.code_loc_offset + 2);
    set_0x10_script_id(script->vram_org, script->header.id);
    set_0x14_script_org_offset(script->vram_org, script->data_org);
    set_0x2e_script_header_word_2(script->vram_org, script->header.unknown01);
    set_0x02_unknown(script->vram_org, 1);
    set_0x01_cstart(script->vram_org, 1);
    set_0x04_cstart_csleep(script->vram_org, 0xff);
    set_0x1a_cforme(script->vram_org, 0xffff);
    set_0x0e_script_ent(script->vram_org, alis.dernent);
    set_0x24_scan_inter(script->vram_org, 2);
    set_0x18_unknown(script->vram_org, 0);
    set_0x0c_vacc_offset(script->vram_org, 0);
    set_0x22_cworld(script->vram_org, 0);
    set_0x20_set_vect(script->vram_org, 0);
    set_0x03_xinv(script->vram_org, 0);
    set_0x25_credon_credoff(script->vram_org, 0);
    set_0x26_creducing(script->vram_org, 0xff);
    set_0x2a_clinking(script->vram_org, 0); // byte
    set_0x2c_calign(script->vram_org, 0);
    set_0x2d_calign(script->vram_org, 0);
    set_0x28_unknown(script->vram_org, 0); // wide
    set_0x2f_chsprite(script->vram_org, 0);
    set_0x32_unknown(script->vram_org, 0); // wide
    set_0x34_unknown(script->vram_org, 0);
    set_0x30_unknown(script->vram_org, 0);

    //set_0x2d_calign(script->vram_org, script->header.unknown02);

    script->pc = script->pc_org = get_0x08_script_ret_offset(script->vram_org);
    
    s16 nextent = xswap16(alis.atent_ptr[caller_idx].offset);
    alis.atent_ptr[caller_idx].offset = xswap16(alis.dernent);
    alis.dernent = xswap16(alis.atent_ptr[script_idx].offset);
    alis.atent_ptr[script_idx].offset = xswap16(nextent);
    alis.atent_ptr[script_idx].vram_offset = xswap32(script->vram_org);
    alis.scripts[script_idx] = script;
    alis.finent += vram_length;
    alis.nbent ++;

    debug(EDebugInfo, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", script->name, script->vram_org, alis.finent, script->vacc_off, script->pc_org);
}


/*
 PACKED SCRIPT FILE FORMAT
 byte(s)    len     role
 -------------------------------------------------------------------------------
 0          1       if high nibble is A, file is packed (b[0] & 0xf0 == 0xa0)
 1..3       3       depacked size
 4...5      2       if zero, this is the main script
 
 (main only)
 6...21     16      main header bytes
 22...29    8       depack dictionary
 30...xx    ?       packed data
 
 (other)
 6...13     8       depack dictionary
 14...xx    ?       packed data

 
 UNPACKED SCRIPT FILE FORMAT
 byte(s)    len     role
 -------------------------------------------------------------------------------
 0...23     24      header bytes
 24...xx    ?       unpacked data (1st word is ID, must be zero)
 */
sAlisScript * script_load(const char * script_path) {
    
    sAlisScript * script = NULL;
    
    FILE * fp = fopen(script_path, "rb");
    if (fp) {
        debug(EDebugInfo,
              "\nLoading script file: %s\n", script_path);
        
        // get packed file size
        fseek(fp, 0L, SEEK_END);
        u32 input_sz = (u32)ftell(fp);
        rewind(fp);

        // read header
        u32 magic = fread32(fp, alis.platform.is_little_endian);
        u16 check = fread16(fp, alis.platform.is_little_endian);
        
        u32 depak_sz = input_sz;
        
        // TODO: check if this was already loaded, if so use cache
        
        // decrunch if needed
        u32 pak_sz = input_sz - HEADER_MAGIC_SZ - HEADER_CHECK_SZ - HEADER_DIC_SZ;
        if(is_main(check)) {
            debug(EDebugInfo, "Main script detected.\n");
            
            // skip vm specs
            fseek(fp, HEADER_MAIN_SZ, SEEK_CUR);
            pak_sz -= HEADER_MAIN_SZ;
        }

        // read dictionary
        u8 dic[HEADER_DIC_SZ];
        fread(dic, sizeof(u8), HEADER_DIC_SZ, fp);
        
        // read file into buffer
        u8 * pak_buf = (u8 *)malloc(pak_sz * sizeof(u8));
        fread(pak_buf, sizeof(u8), pak_sz, fp);
        
        // alloc and depack
        debug(EDebugInfo, "Depacking...\n");
        depak_sz = get_depacked_size(magic);
        u8 * depak_buf = (u8 *)malloc(depak_sz * sizeof(u8));
        
        depak_sz = unpack_script(script_path, alis.platform.is_little_endian, &depak_buf);
        if (depak_sz > 0) {
            // unpacked, continue
            debug(EDebugInfo,
                  "Depacking done in %ld bytes (~%d%% packing ratio)\n",
                  depak_sz, 100 - (100 * pak_sz) / depak_sz);
            
            // init script
            script = script_init(strrchr(script_path, kPathSeparator) + 1, depak_buf, depak_sz);
        }
        else {

            // not packed !!
            debug(EDebugFatal,
                  "Unpacked scripts are not supported: '%s'\n",
                  script_path);
        }

        // cleanup
        free(depak_buf);
        free(pak_buf);

        fclose(fp);
    }
    else {
        debug(EDebugFatal,
              "Failed to open script at path '%s'\n",
              script_path);

    }
    return script;
}


void script_unload(sAlisScript * script) {
//    free(script->ram);
//    free(script->data);
    free(script);
}


// =============================================================================
// MARK: - Script data access
// =============================================================================
void script_read_debug(s32 value, size_t sz) {
    switch (sz) {
        case 1:
            debug(EDebugInfo, " 0x%02x", value & 0xff);
            break;
        case 2:
            debug(EDebugInfo, " 0x%04x", value & 0xffff);
            break;
        case 4:
            debug(EDebugInfo, " 0x%06x", value & 0xffffff);
            break;
        default:
            debug(EDebugInfo, " %d", value);
            break;
    }
}

u8 script_read8(void) {
    u8 ret = (alis.mem[alis.script->pc++]);
    script_read_debug(ret, sizeof(u8));
    return ret;
}

/**
 * @brief Reads a word from current script
 * 
 * @return u16 
 */
u16 script_read16(void) {
    
    u16 ret = read16(alis.mem + alis.script->pc, alis.platform.is_little_endian);
    alis.script->pc += 2;
    script_read_debug(ret, sizeof(u16));
    return ret;
}

u32 script_read24(void) {
    u32 ret = read24(alis.mem + alis.script->pc, alis.platform.is_little_endian);
    alis.script->pc += 3;
    script_read_debug(ret, sizeof(u32));
    return ret;
}

void script_read_bytes(u32 len, u8 * dest) {
    while(len--) {
        *dest++ = alis.mem[alis.script->pc++];
    }
}

void script_read_until_zero(u8 * dest) {
    while((*dest++ = alis.mem[alis.script->pc++]));
}

void script_jump(s32 offset) {
    if(!alis.disasm) {
        alis.script->pc += offset;
    }
}


void script_debug(sAlisScript * script) {
    
    printf("\n-- SCRIPT --\n'%s' (0x%02x)\nHeader:\n",
           script->name,
           script->header.id);
    
    // total header len is located in header, also add sizeof(script_id)
    u8 header_len = script->header.code_loc_offset + sizeof(u16) /* script ID length */;
    
//    for(int i = 0; i < header_len; i++) {
//        printf("%02x ", script->data_org[i]);
//    }
    
    u8 code = *(alis.mem + alis.script->pc++);//*(script->pc);
    printf("\nDATA ORG: 0x%06x\nCODE ORG: 0x%06x\nPC OFFSET: 0x%04x\nPC BYTE: 0x%02x ('%s')\n",
           script->data_org,
           script->data_org + header_len,
           alis.script->pc,
           // script_pc(script),
           code,
           opcodes[code].name);
}

u16 get_0x34_unknown(u32 vram)                          { return xread16(vram - 0x34); }
u8 get_0x32_unknown(u32 vram)                           { return xread8(vram - 0x32); }
u8 get_0x31_unknown(u32 vram)                           { return xread8(vram - 0x31); }
u8 get_0x30_unknown(u32 vram)                           { return xread8(vram - 0x30); }
u8 get_0x2f_chsprite(u32 vram)                          { return xread8(vram - 0x2f); }
u8 get_0x2e_script_header_word_2(u32 vram)              { return xread8(vram - 0x2e); }
u8 get_0x2d_calign(u32 vram)                            { return xread8(vram - 0x2d); }
u8 get_0x2c_calign(u32 vram)                            { return xread8(vram - 0x2c); }
u8 get_0x2b_cordspr(u32 vram)                           { return xread8(vram - 0x2b); }
u16 get_0x2a_clinking(u32 vram)                         { return xread16(vram - 0x2a); }
u8 get_0x28_unknown(u32 vram)                           { return xread8(vram - 0x28); }
u8 get_0x27_creducing(u32 vram)                         { return xread8(vram - 0x27); }
u8 get_0x26_creducing(u32 vram)                         { return xread8(vram - 0x26); }
u8 get_0x25_credon_credoff(u32 vram)                    { return xread8(vram - 0x25); }
s8 get_0x24_scan_inter(u32 vram)                        { return xread8(vram - 0x24); }
u8 get_0x23_unknown(u32 vram)                           { return xread8(vram - 0x23); }
u16 get_0x22_cworld(u32 vram)                           { return xread16(vram - 0x22); }
//u8 get_0x21_cworld(u32 vram)                            { return xread8(vram - 0x21); }
u16 get_0x20_set_vect(u32 vram)                         { return xread16(vram - 0x20); }
u16 get_0x1e_scan_clr(u32 vram)                         { return xread16(vram - 0x1e); }
u16 get_0x1c_scan_clr(u32 vram)                         { return xread16(vram - 0x1c); }
s16 get_0x1a_cforme(u32 vram)                           { return xread16(vram - 0x1a); }
u16 get_0x18_unknown(u32 vram)                          { return xread16(vram - 0x18); }
u16 get_0x16_screen_id(u32 vram)                        { return xread16(vram - 0x16); }
u32 get_0x14_script_org_offset(u32 vram)                { return xread32(vram - 0x14); }
u16 get_0x10_script_id(u32 vram)                        { return xread16(vram - 0x10); }
u16 get_0x0e_script_ent(u32 vram)                       { return xread16(vram - 0xe); }
u16 get_0x0c_vacc_offset(u32 vram)                      { return xread16(vram - 0xc); }
u16 get_0x0a_vacc_offset(u32 vram)                      { return xread16(vram - 0xa); }
u32 get_0x08_script_ret_offset(u32 vram)                { return xread32(vram - 0x8); }
u8 get_0x04_cstart_csleep(u32 vram)                     { return xread8(vram - 0x4); }
u8 get_0x03_xinv(u32 vram)                              { return xread8(vram - 0x3); }
u8 get_0x02_wait_cycles(u32 vram)                           { return xread8(vram - 0x2); }
u8 get_0x01_wait_count(u32 vram)                            { return xread8(vram - 0x1); }

void set_0x34_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x34, val); }
void set_0x32_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x32, val); }
void set_0x31_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x31, val); }
void set_0x30_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x30, val); }
void set_0x2f_chsprite(u32 vram, u8 val)                { xwrite8(vram - 0x2f, val); }
void set_0x2e_script_header_word_2(u32 vram, u8 val)    { xwrite8(vram - 0x2e, val); }
void set_0x2d_calign(u32 vram, u8 val)                  { xwrite8(vram - 0x2d, val); }
void set_0x2c_calign(u32 vram, u8 val)                  { xwrite8(vram - 0x2c, val); }
void set_0x2b_cordspr(u32 vram, u8 val)                 { xwrite8(vram - 0x2b, val); }
void set_0x2a_clinking(u32 vram, u16 val)               { xwrite16(vram - 0x2a, val); }
void set_0x28_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x28, val); }
void set_0x27_creducing(u32 vram, u8 val)               { xwrite8(vram - 0x27, val); }
void set_0x26_creducing(u32 vram, u8 val)               { xwrite8(vram - 0x26, val); }
void set_0x25_credon_credoff(u32 vram, u8 val)          { xwrite8(vram - 0x25, val); }
void set_0x24_scan_inter(u32 vram, s8 val)              { xwrite8(vram - 0x24, val); }
void set_0x23_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x23, val); }
void set_0x22_cworld(u32 vram, u16 val)                 { xwrite16(vram - 0x22, val); }
//void set_0x21_cworld(u32 vram, u8 val)                  { xwrite8(vram - 0x21, val); }
void set_0x20_set_vect(u32 vram, u16 val)               { xwrite16(vram - 0x20, val); }
void set_0x1e_scan_clr(u32 vram, u16 val)               { xwrite16(vram - 0x1e, val); }
void set_0x1c_scan_clr(u32 vram, u16 val)               { xwrite16(vram - 0x1c, val); }
void set_0x1a_cforme(u32 vram, s16 val)                 { xwrite16(vram - 0x1a, val); }
void set_0x18_unknown(u32 vram, u16 val)                { xwrite16(vram - 0x18, val); }
void set_0x16_screen_id(u32 vram, u16 val)              { xwrite16(vram - 0x16, val); }
void set_0x14_script_org_offset(u32 vram, u32 val)      { xwrite32(vram - 0x14, val); }
void set_0x10_script_id(u32 vram, u16 val)              { xwrite16(vram - 0x10, val); }
void set_0x0e_script_ent(u32 vram, u16 val)             { xwrite16(vram - 0x0e, val); }
void set_0x0c_vacc_offset(u32 vram, u16 val)            { xwrite16(vram - 0x0c, val); }
void set_0x0a_vacc_offset(u32 vram, u16 val)            { xwrite16(vram - 0x0a, val); }
void set_0x08_script_ret_offset(u32 vram, u32 val)      { xwrite32(vram - 0x08, val); }
void set_0x04_cstart_csleep(u32 vram, u8 val)           { xwrite8(vram - 0x04, val); }
void set_0x03_xinv(u32 vram, u8 val)                    { xwrite8(vram - 0x03, val); }
void set_0x02_unknown(u32 vram, u8 val)                 { xwrite8(vram - 0x02, val); }
void set_0x01_cstart(u32 vram, u8 val)                  { xwrite8(vram - 0x01, val); }
