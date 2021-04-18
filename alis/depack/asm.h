//
//  asm.h
//  silm-depack
//

#ifndef asm_h
#define asm_h

#include "../config.h"

extern uint8_t d0[4];
extern uint8_t d1[4];
extern uint8_t d2[4];
extern uint8_t d3[4];
extern uint8_t d4[4];
extern uint8_t d5[4];
extern uint8_t d6[4];
extern uint8_t d7[4];

extern uint8_t * ram;

extern uint8_t * a0;
extern uint8_t * a1;
extern uint8_t * a2;
extern uint8_t * a3;
extern uint8_t * a4;
extern uint8_t * a5;
extern uint8_t * a6;
extern uint8_t * a7;

typedef enum {
    B = 8,
    W = 16,
    L = 32
} ESz;


uint8_t BYTE(uint8_t reg[4]);
uint16_t WORD(uint8_t reg[4]);
uint32_t LONG(uint8_t reg[4]);

void MOVE(ESz sz, u16 src_inc, u8 * src_addr, u8 * dest);

void ANDI_W(uint16_t val, uint8_t reg[4]);
void ADDQ_W(uint16_t val, uint8_t reg[4]);
void ADDREG_W(uint8_t sreg[4], uint8_t dreg[4]);
void ADDREG_B(uint8_t sreg[4], uint8_t dreg[4]);

void CLR_W(uint8_t * addr);
void CLRREG_W(uint8_t reg[4]);

void EXT_W(uint8_t reg[4]);

void MOVE(ESz sz, u16 src_inc, u8 * src_addr, u8 * dest);
void MOVE_B(uint8_t val, uint8_t reg[4]);
void MOVE_W(uint16_t val, uint8_t reg[4]);
void MOVE_L(uint32_t val, uint8_t reg[4]);
void MOVEQ(uint8_t val, uint8_t reg[4]);
void MOVEREG_W(uint8_t sreg[4], uint8_t dreg[4]);

void MULSREG_W(uint8_t sreg[4], uint8_t dreg[4]);

void NEG_W(uint8_t reg[4]);

void ROLREG_L(uint8_t offsetreg[4], uint8_t dreg[4]);



void SUBREG_B(uint8_t sreg[4], uint8_t dreg[4]);
void SUBREG_W(uint8_t sreg[4], uint8_t dreg[4]);
void SUB_L(uint32_t val, uint8_t dreg[4]);
void SUB_W(uint16_t val, uint8_t dreg[4]);

void SWAP(uint8_t reg[4]);

void dreg(void);


#endif /* asm_h */
