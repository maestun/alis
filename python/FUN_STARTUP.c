
#include <stdint.h>

// global shite
const uint32_t kAddrAtent = 0x224f0;
uint16_t save_rsp;
uint8_t fallent, fseq;

const char kScriptHeaderInterruptOffset = 0xa;
const char kScriptHeaderSubroutineOffset = 0x6;

char kByte_Minus0x01 = -0x01; // -1
char kByte_Minus0x02 = -0x02; // -2
char kByte_Minus0x04 = -0x04; // -4
char kLong_SaveVirtualProgramCounter_Minus0x08 = -0x08; // -8
char kWord_SaveVirtualAccOffset_Minus0x0a = -0x0a;      // -10
char kLong_ScriptDataStart_Minus0x14 = -0x14;           // -20
char kByte_ScanInterFlags_Minus0x24 = -0x24;            // -36


void FUN_STARTUP(void) {
    uint32_t a3_virtual_pc;
    uint16_t d4_virtual_acc_offset;

    int16_t w_offset_atent;
    
    uint32_t addr_current_ctx; // A0 et A6 sont utilisés
    
    
    // init shite, on y passe qu'une fois
    //
    // addr_buffer256_D7 = bsd7_256;
    // addr_buffer256_D6 = bsd6_256;
    // addr_old_buffer256_D7 = bsd7bis;
    // addr_tpalet = &tpalet;
    // addr_mpalet = &mpalet;
    // ptabfen = tabfen_640;
    // ptrent = tablent_256;
    // save_sp = (undefined *)register0x0000003c;
    // SYS_Init();
    // FUN_InitChannels();
    // FUN_BootMain();

__reprog:

    w_offset_atent = 0;
    // inutile
    // addr_current_ctx = kAddrAtent;
    
    // load main script

    while(1) {
        // addr_current_ctx = *(int *)(addr_current_ctx + w_offset_atent);
__moteur2:
        // a0 is used
        addr_current_ctx = *(int *)(kAddrAtent + w_offset_atent);
        fallent = 0;
        fseq = 0;

        // execute interrupt routine ?
        if (// test context: scan/inter/send flags
            ((*(char *)(addr_current_ctx + kByte_ScanInterFlags_Minus0x24) < 0) &&
            // test context: bit 2 is INTERRUPT bit (0 == interrupt)
            ((*(char *)(addr_current_ctx + kByte_ScanInterFlags_Minus0x24) & 2) == 0)) &&
            // check for interrupt routine offset in current script's header (at offset 0x0a)
            (*(int *)(*(int *)(addr_current_ctx + kLong_ScriptDataStart_Minus0x14) + kScriptHeaderInterruptOffset) != 0)) {
            
__before_moteur4:
            save_rsp = *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a);
            savecoord();
            FUN_READEXEC_OPCODE();
            updtcoo0();
        }
    
__moteur4:
        // execute program flow
        // TODO: what is -4 in ctx ??
        if (*(char *)(addr_current_ctx + kByte_Minus0x04) != 0) {

            if (*(char *)(addr_current_ctx + kByte_Minus0x04) < 0) {
                // is negative => set to 1
                *(char *)(addr_current_ctx + kByte_Minus0x04) = 1;
            }
            
            // TODO: what is -1 in ctx ??
            char pcVar3 = (char *)(addr_current_ctx + kByte_Minus0x01);
            if (--pcVar3 == 0) {

                savecoord();
                // restore A3 and D4 registers, they're lobal and used by FUN_READEXEC_OPCODE
                a3_virtual_pc = *(uint32_t *)(addr_current_ctx + kLong_SaveVirtualProgramCounter_Minus0x08);
                d4_virtual_acc_offset = *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a);
                fseq++;
                FUN_READEXEC_OPCODE();
                
                // save A3 and D4 registers into context
                *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a) = d4_virtual_acc_offset;
                *(uint32_t *)(addr_current_ctx + kLong_SaveVirtualProgramCounter_Minus0x08) = a3_virtual_pc;
                
                // execute subroutine ?
                // get start addr of script data, skip 6 bytes, to get offset to an optional subroutine
                if (*(int *)(*(int *)(addr_current_ctx + kLong_ScriptDataStart_Minus0x14) + kScriptHeaderSubroutineOffset) != 0) {
__execute_subroutine:
                    // subroutine found
                    fseq = 0;
                    save_rsp = *(uint16_t *)(addr_current_ctx + kWord_SaveVirtualAccOffset_Minus0x0a);
                    FUN_READEXEC_OPCODE();
                }
                updtcoo0();
                *(char *)(addr_current_ctx + kByte_Minus0x01) = *(char *)(addr_current_ctx + kByte_Minus0x02);
            }
        }

__moteur1:
        // TODO: c'est ici qu'on maj D5 ?
        w_offset_atent = *(short *)(kAddrAtent + 4 + (int)w_offset_atent);
        
        // inutile
        // addr_current_ctx = kAddrAtent;

        // blit !
        if (w_offset_atent == 0) {
            image();
        }
  }
}