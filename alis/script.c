//
//  script.c
//  alis
//

#include "alis.h"
#include "depack/asm.h"
#include "debug.h"
#include "platform.h"
#include "script.h"
#include "utils.h"
#include "alis_private.h"



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
#define HEADER_MAGIC_LEN_SZ (sizeof(u32))
// byte 4..5 -> main script if zero
#define HEADER_CHECK_SZ     (sizeof(u16))
// if main: byte 6..21 -> main header
#define HEADER_MAIN_SZ      (16 * sizeof(u8))
// if main: byte 22..29 -> dic
// if not main: byte 6..13 -> dic
#define HEADER_DIC_SZ       (2 * sizeof(u32))

// TODO: valid only for big endian
u8 is_packed(u8 * pak_buffer) {
    return (*pak_buffer & 0xf0) == 0xa0;
}

u32 get_depacked_size(u8 * pak_buffer) {
    return read_big_endian(pak_buffer + 1, sizeof(u16) + sizeof(u8));// (pak_buffer[1] << 16) + (pak_buffer[2] << 8) + pak_buffer[3];
}

int is_main(u8 * pak_buffer) {
    return kMainScriptID == read_big_endian(pak_buffer + HEADER_MAGIC_LEN_SZ, sizeof(u16));
}


void depak_11() {
//    printf("depak_11 ($163a8)\n");
    ADDREG_B(d0, d7);               // ADD.B     D0,D7
    CLRREG_W(d5);                      // CLR.W     D5
    ROLREG_L(d7, d5);               // ROL.L     D7,D5
    SWAP(d5);                       // SWAP      D5
//    if(p_pak > p_pak_end) {         // CMPA.L    A4,A0
//        get_moar_data(); // BGE       _DEPACK_GET_MORE_DATA ; read 32k bytes
//    }

// _DEPACK_12:
    uint16_t w = *p_pak;
    w <<= 8;
    w += *(p_pak + 1);
//    printf("depak11: read word 0x%x at address 0x%x\n", w, (uint32_t)p_pak);
    p_pak += 2;
    MOVE_W(w, d5);                  // MOVE.W    (A0)+,D5
//    MOVE(W, 0, p_pak++, d5)

    SWAP(d5);                       // SWAP      D5

    SUBREG_B(d7, d0);               // SUB.B     D7,D0
    MOVEQ(16, d7);                  // MOVEQ     #$10,D7
    ROLREG_L(d0, d5);               // ROL.L     D0,D5

    SUBREG_B(d0, d7);               // SUB.B     D0,D7
}


void depak_2() {
//    printf("depak_2 ($1639e)\n");
    SUBREG_B(d0, d7);               // SUB.B     D0,D7
    if((int8_t)BYTE(d7) < 0) {      // BMI.S     _DEPACK_11
        depak_11();
    }
    else {
        CLRREG_W(d5);                      // CLR.W     D5
        ROLREG_L(d0, d5);               // ROL.L     D0,D5
    }
}


void depak(uint8_t * aPakBuffer, // A0
           uint8_t * aDepakBuffer, // A1 -> pointe sur byte courant, A2 pointe sur dernier byte
           size_t aPakSize,
           size_t aDepakSize, // D1
           uint8_t * aDic) { // A5

//    printf("DEPACK: unpacking...\n");
//    printf("PAK buffer start: 0x%08x\n", (uint32_t) aPakBuffer);
//    printf("PAK buffer size: %ld\n", aPakSize);
//    printf("PAK buffer end: 0x%08x\n", (uint32_t)(aPakBuffer + aPakSize));
//    printf("DEPAK buffer start: 0x%08x\n", (uint32_t)aDepakBuffer);
//    printf("DEPAK buffer size: %ld\n", aDepakSize);
//    printf("DEPAK buffer end: 0x%08x\n", (uint32_t)(aDepakBuffer + aDepakSize));

    int16_t offset = 0;
    uint8_t tmp_b = 0;

    p_depak = aDepakBuffer; // A1
    p_depak_end = aDepakBuffer + aDepakSize - 1; // A2
    p_pak = aPakBuffer; // A0
    p_pak_end = aPakBuffer + aPakSize; // A4
    p_dic = aDic;

    CLRREG_W(d7);

_depak_start:
//    printf("depak_start ($1632e)\n");
    if(p_depak > p_depak_end) {     // CMPA.L    A2,A1
        goto _depak_end;            // BGT       FN_DEPACK_END ; si a2 (adresse fin decrunch) > a1 (adresse debut decrunch) alors fin
    }
    MOVEQ(1, d0);                   // MOVEQ     #1,D0
    depak_2();                      // BSR.S     _DEPACK_2
    if(BYTE(d5) == 0) {             // TST.B     D5
        goto _depak_3;              // BEQ.S     _DEPACK_3
    }
    MOVEQ(0, d2);                   // MOVEQ     #0,D2

_depak_4:
//    printf("depak_4 ($1633e)\n");
    MOVEQ(2, d0);                   // MOVEQ     #2,D0
    depak_2();                      // BSR.S     _DEPACK_2
    ADDREG_W(d5, d2);               // ADD.W     D5,D2
    if(WORD(d5) == 3) {             // CMP.W     #3,D5
        goto _depak_4;              // BEQ.S     _DEPACK_4
    }

_depak_5:
//    printf("depak_5 ($1634a)\n");
    MOVEQ(8, d0);                   // MOVEQ     #8,D0
    depak_2();                      // BSR.S     _DEPACK_2
    *p_depak = BYTE(d5);            // MOVE.B    D5,(A1)+      ; ecriture d'un octet depack :)

//    printf("depak5: wrote %uth byte (0x%02x) at address 0x%08x\n", depak_counter++, BYTE(d5), (uint32_t)p_depak);

    p_depak++;
    SUB_W(1, d2);
    if((int16_t)WORD(d2) >= 0) {             // DBF       D2,_DEPACK_5
        goto _depak_5;
    }

    if(p_depak > p_depak_end) {     // CMPA.L    A2,A1
        goto _depak_end;            // BGT       FN_DEPACK_END ; si a2 (adresse fin decrunch) > a1 (adresse debut decrunch) alors fin
    }

_depak_3:
//    printf("depak_3 ($1635a)\n");
    MOVEQ(3, d0);                   // MOVEQ     #3,D0
    depak_2();
    CLRREG_W(d0);                      // CLR.W     D0

//    printf("Dic: byte %d is 0x%02x\n", WORD(d5), p_dic[WORD(d5)]);
    MOVE_B(p_dic[WORD(d5)], d0);    // MOVE.B    0(A5,D5.W),D0

    ANDI_W(3, d5);                  // ANDI.W    #3,D5
    if(WORD(d5) == 0) {
        goto _depak_7;              // BEQ       _DEPACK_7
    }
//    MOVEREG_W(d5, d2);              // MOVE.W    D5,D2
    MOVE(W, 0, d5, d2);

    depak_2();                      // BSR.S     _DEPACK_2

_depak_8:
//    printf("depak_8 ($1636e)\n");
    NEG_W(d5);                      // NEG.W     D5

_depak_9:

    // MOVE.B    -1(A1,D5.W),(A1)+ ; ecriture d'un octet depack :)
    offset = WORD(d5) * -1;
    offset++;
    tmp_b = *(p_depak - offset);
    *p_depak = tmp_b;
//    printf("depak9: wrote %uth byte (0x%02x) at address 0x%08x\n", depak_counter++, tmp_b, (uint32_t)p_depak);
    p_depak++;


    SUB_W(1, d2);
    if((int16_t)WORD(d2) >= 0) {    // DBF       D2,_DEPACK_9
        goto _depak_9;
    }
    goto _depak_start;              // BRA       _DEPACK_START

_depak_7:
//    printf("depak_7\n");
    depak_2();                      // BSR       _DEPACK_2
    // MOVEREG_W(d5, d3);              // MOVE.W    D5,D3
    MOVE(W, 0, d5, d3);

    CLRREG_W(d2);                      // CLR.W     D2

_depak_10:
//    printf("depak_10\n");
    MOVEQ(3, d0);                   // MOVEQ     #3,D0
    depak_2();                      // BSR.S     _DEPACK_6

    ADDREG_W(d5, d2);               // ADD.W     D5,D2
    if(WORD(d5) == 7) {             // CMP.W     #7,D5
        goto _depak_10;             // BEQ.S     _DEPACK_10
    }

//    MOVEREG_W(d3, d5);              // MOVE.W    D3,D5
    MOVE(W, 0, d3, d5);

    ADDQ_W(4, d2);                  // ADDQ.W    #4,D2

    goto _depak_8;                  // BRA.S     _DEPACK_8

_depak_end:
//    printf("depak_end\n");
    return;
}


// =============================================================================
// MARK: - Script API
// =============================================================================

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
        debug(EDebugVerbose,
              "\nLoading script file: %s\n", script_path);
        
        // get script file size
        fseek(fp, 0L, SEEK_END);
        u32 pak_sz = (u32)ftell(fp);
        rewind(fp);

        // read file into buffer
        u8 * pak_buf = malloc(pak_sz * sizeof(u8));
        fread(pak_buf, sizeof(u8), pak_sz, fp);
        
        u8 main = 0;
        u32 depak_sz = pak_sz;
        
        // TODO: check if this was already loaded, if so use cache

        // decrunch if needed
        u8 * data = pak_buf;
        if(is_packed(pak_buf)) {
            debug(EDebugVerbose, "Unpacking file...\n");
            
            if(is_main(pak_buf)) {
                debug(EDebugVerbose, "Main script detected:\n");
                main = 1;
                
                // main script: packed header contains vm info !
                alis_config_vm(pak_buf + HEADER_MAGIC_LEN_SZ + HEADER_CHECK_SZ);
            }

            // alloc and depack
            depak_sz = get_depacked_size(pak_buf);
            u8 dic_offset = HEADER_MAGIC_LEN_SZ +
                            HEADER_CHECK_SZ +
                            (main ? HEADER_MAIN_SZ : 0);
            u8 pak_offset = dic_offset + HEADER_DIC_SZ;
            u8 * depak_buf = malloc(depak_sz * sizeof(u8));
            depak(pak_buf + pak_offset,
                  depak_buf,
                  pak_sz - pak_offset,
                  depak_sz,
                  &pak_buf[dic_offset]);
            
            debug(EDebugVerbose,
                       "Unpacking done in %ld bytes (~%d%% packing ratio)\n",
                       depak_sz, 100 - (100 * pak_sz) / depak_sz);
            
            // cleanup
            data = depak_buf;
            free(pak_buf);
            pak_buf = NULL;
        }
        else {
            // not packed !!
        }
        
        // init script
        script = (sAlisScript *)malloc(sizeof(sAlisScript));
        strcpy(script->name, strrchr(script_path, kPathSeparator) + 1);
        *(strrchr(script->name, '.')) = '\0';
        script->sz = depak_sz;
        
        // script data (don't use memcpy cuz little endian)
        // TODO: endianness dependency
        script->header.id = read_big_endian(data + 0, sizeof(u16));
        script->header.w_0x1700 = read_big_endian(data + 2, sizeof(u16));
        script->header.code_loc_offset = read_big_endian(data + 4, sizeof(u16));
        script->header.ret_offset = read_big_endian(data + 6, sizeof(u32));
        script->header.dw_unknown3 = read_big_endian(data + 10, sizeof(u32));
        script->header.dw_unknown4 = read_big_endian(data + 14, sizeof(u32));
        script->header.w_unknown5 = read_big_endian(data + 18, sizeof(u16));
        script->header.vram_alloc_sz = read_big_endian(data + 20, sizeof(u16));
        script->header.w_unknown7 = read_big_endian(data + 22, sizeof(u16));
        
        // TODO: this is for debug / static allocs
        sScriptDebug debug_data = script_debug_data[script->header.id];

        // tell where the script vram is located in host memory
        script->vram_org = debug_data.vram_org; // TODO: for main it's $2261c (DAT_0001954c) + header_word5 + header_word7 + 0x34 (sizeof(context))
        u32 test = sizeof(script->context) + script->header.w_unknown5 + script->header.w_unknown7;
        script->vacc_off = debug_data.vacc_off;
        script->data_org = debug_data.data_org;
        
        // init context
        memset(&(script->context), 0, sizeof(script->context));
        script->context._0x10_script_id = script->header.id;
        
        script->context._0x14_script_org_offset = script->data_org;
        script->context._0x8_script_ret_offset = script->data_org + script->header.code_loc_offset + 2;
        script->context._0x2e_script_header_word_2 = script->header.w_0x1700;
        script->context._0x2_unknown = 1;
        script->context._0x1_cstart = 1;
        script->context._0x4_cstart_csleep = 0xff;
        script->context._0x1a_cforme = 0xff;
        script->context._0x24_scan_inter.inter_off_bit_1 = 1;
        script->context._0x24_scan_inter.scan_off_bit_0 = 1;
        script->context._0x26_creducing = 0xff;
        
        // copy script data to static host memory
        memcpy(alis.mem + script->data_org, data, depak_sz);
        
        // script program counter starts kScriptHeaderLen after data
        script->pc = script->pc_org = script->context._0x8_script_ret_offset; //(script->data_org + kScriptHeaderLen);
        
        debug(EDebugVerbose,
              "Script '%s' loaded (ID = 0x%02x)\nVRAM at address 0x%x\nDATA at address 0x%x\nCODE at address 0x%x\nVACC = 0x%04x\n",
              script->name, script->header.id,
              script->vram_org,
              script->data_org,
              script->pc_org,
              script->vacc_off);

        // cleanup
        free(data);
        fclose(fp);
    }
    else {
        debug(EDebugFatal,
              "Failed to load script at path '%s'\n",
              script_path);

    }
    return script;
}

void script_unload(sAlisScript * script) {
//    free(script->ram);
//    free(script->data);
    free(script);
}


//u32 script_pc(sAlisScript * script) {
//    return (u32)(script->pc - alis.mem);
//}


// =============================================================================
// MARK: - Script data access
// =============================================================================
void script_read_debug(s32 value) {
    if (value > 0xffff) {
        debug(EDebugVerbose, " 0x%06x", value);
    }
    else if (value > 0xff) {
        debug(EDebugVerbose, " 0x%04x", value);
    }
    else if (value > 0x0) {
        debug(EDebugVerbose, " 0x%02x", value);
    }
    else {
        debug(EDebugVerbose, " %d", value);
    }
}

u8 script_read8(void) {
    u8 ret = *(alis.mem + alis.script->pc++);
    script_read_debug(ret);
    return ret;
}

s16 script_read8ext16(void) {
    u8 b = *(alis.mem + alis.script->pc++);
    s16 ret = b;
    if(BIT_CHK((b), 7)) {
        ret |= 0xff00;
    }
    
    script_read_debug(ret);
    return  ret;
}

s32 script_read8ext32(void) {
    s16 ret = extend_l(extend_w(*(alis.mem + alis.script->pc++)));
    script_read_debug(ret);
    return  ret;
}

u16 script_read16(void) {
    u16 ret = read_big_endian((alis.mem + alis.script->pc++), sizeof(u16)); // (*alis.script->pc++ << 8) + *alis.script->pc++;
    script_read_debug(ret);
    return ret;
}

s32 script_read16ext32(void) {
    u32 ret = extend_l(read_big_endian((alis.mem + alis.script->pc++), sizeof(u16)));
    script_read_debug(ret);
    return ret;
}

u32 script_read24(void) {
    u32 ret = read_big_endian((alis.mem + alis.script->pc++), sizeof(u16) + sizeof(u8)); // (*alis.script->pc++ << 16) + (*alis.script->pc++ << 8) + *alis.script->pc++;
    script_read_debug(ret);
    return ret;
}

void script_read_bytes(u32 len, u8 * dest) {
    while(len--) {
        *dest++ = *(alis.mem + alis.script->pc++);
    }
}

void script_read_until_zero(u8 * dest) {
    while(*(alis.mem + alis.script->pc++)) {
        *dest++ = *(alis.mem + alis.script->pc++);
    }
    alis.script->pc++;
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

