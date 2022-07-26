//
//  asm.c
//  silm-depack
//

#include "asm.h"

uint8_t d0[4] = {0};
uint8_t d1[4] = {0};
uint8_t d2[4] = {0};
uint8_t d3[4] = {0};
uint8_t d4[4] = {0};
uint8_t d5[4] = {0};
uint8_t d6[4] = {0};
uint8_t d7[4] = {0};

uint8_t * a0;
uint8_t * a1;
uint8_t * a2;
uint8_t * a3;
uint8_t * a4;
uint8_t * a5;
uint8_t * a6;
uint8_t * a7;

uint8_t * ram;

uint8_t BYTE(uint8_t reg[4]) {
    return reg[0];
}

uint16_t WORD(uint8_t reg[4]) {
    return ((reg[1] << 8) + reg[0]);
}

uint32_t LONG(uint8_t reg[4]) {
    return ((reg[3] << 24) + (reg[2] << 16) + (reg[1] << 8) + reg[0]);
}

void CLR_W(uint8_t * addr) {
    *addr = 0;
}

void CLRREG_W(uint8_t reg[4]) {
    reg[0] = reg[1] = 0;
}

void MOVE_B(uint8_t val, uint8_t reg[4]) {
    reg[0] = val;
}

void MOVE_W(uint16_t val, uint8_t reg[4]) {
    uint8_t b1 = ((val >> 8) & 0xff);
    uint8_t b0 = (val & 0xff);
    reg[1] = b1;
    reg[0] = b0;
}

void MOVE_L(uint32_t val, uint8_t reg[4]) {
    uint8_t b3 = ((val >> 24) & 0xff);
    uint8_t b2 = ((val >> 16) & 0xff);
    uint8_t b1 = ((val >> 8) & 0xff);
    uint8_t b0 = (val & 0xff);
    reg[3] = b3;
    reg[2] = b2;
    reg[1] = b1;
    reg[0] = b0;
}


void ADDREG_W(uint8_t sreg[4], uint8_t dreg[4]) {
    uint16_t w = WORD(sreg);
    w += WORD(dreg);
    MOVE_W(w, dreg);
}

void ADDREG_B(uint8_t sreg[4], uint8_t dreg[4]) {
    uint8_t b = BYTE(sreg);
    b += BYTE(dreg);
    MOVE_B(b, dreg);
}

void MOVEREG_W(uint8_t sreg[4], uint8_t dreg[4]) {
    dreg[1] = sreg[1];
    dreg[0] = sreg[0];
}

void MOVEQ(u8 val, u8 * reg) {
    for(int i = 0; i < 4; i++)
        reg[i] = 0;
    reg[0] = val;
}

void MOVE(ESz sz, u16 src_inc, u8 * src_addr, u8 * dest) {
    switch (sz) {
        case B:
            *(u8 *)dest = *(u8 *)(src_addr + src_inc);
            break;
        case W:
            *(u16 *)dest = *(u16 *)(src_addr + src_inc);
            break;
        case L:
            *(u32 *)dest = *(u32 *)(src_addr + src_inc);
            break;
    }
}


void ANDI_W(uint16_t val, uint8_t reg[4]) {
    uint16_t w = WORD(reg);
    w &= val;
    MOVE_W(w, reg);
}

void ADDQ_W(uint16_t val, uint8_t reg[4]) {
    uint16_t w = WORD(reg);
    w += val;
    MOVE_W(w, reg);
}

void SUBREG_B(uint8_t sreg[4], uint8_t dreg[4]) {
    int8_t b = BYTE(dreg) - BYTE(sreg);
    MOVE_B(b, dreg);
}

void SUBREG_W(uint8_t sreg[4], uint8_t dreg[4]) {
    int16_t b = WORD(dreg) - WORD(sreg);
    MOVE_W(b, dreg);
}

void SUB_L(uint32_t val, uint8_t dreg[4]) {
    int32_t l = LONG(dreg);
    l -= val;
    MOVE_L(l, dreg);
}

void SUB_W(uint16_t val, uint8_t dreg[4]) {
    int16_t w = WORD(dreg);
    w -= val;
    MOVE_W(w, dreg);
}

uint32_t _rotl(const uint32_t value, uint8_t shift) {
    if ((shift &= sizeof(value)*8 - 1) == 0)
        return value;
    return (value << shift) | (value >> (sizeof(value)*8 - shift));
}

void ROLREG_L(uint8_t offsetreg[4], uint8_t dreg[4]) {
    uint32_t offset = LONG(offsetreg);
    uint32_t l = LONG(dreg);
    //l <<= offset;
    l = _rotl(l, offset);
    MOVE_L(l, dreg);
}

void NEG_W(uint8_t reg[4]) {
    uint16_t w = WORD(reg);
    w *= -1;
    MOVE_W(w, reg);
}


void SWAP(uint8_t reg[4]) {
    uint8_t b0 = reg[0];
    uint8_t b1 = reg[1];
    uint8_t b2 = reg[2];
    uint8_t b3 = reg[3];
    reg[0] = b2;
    reg[1] = b3;
    reg[2] = b0;
    reg[3] = b1;
}

void EXT_W(uint8_t reg[4]) {
    if(BIT_CHK(reg[0], 7)) {
        for(int i = 0; i < 8; i++) {
            BIT_SET(reg[1], i);
        }
    }
    else {
        for(int i = 0; i < 8; i++) {
            BIT_CLR(reg[1], i);
        }
    }
}


void MULSREG_W(uint8_t sreg[4], uint8_t dreg[4]) {
    u16 s = WORD(sreg);
    u16 d = WORD(dreg);
    MOVE_W(s * d, dreg);
}


void dreg() {
    printf("D0\t %02x %02x %02x %02x\n", d0[3], d0[2], d0[1], d0[0]);
    printf("D2\t %02x %02x %02x %02x\n", d2[3], d2[2], d2[1], d2[0]);
    printf("D3\t %02x %02x %02x %02x\n", d3[3], d3[2], d3[1], d3[0]);
    printf("D5\t %02x %02x %02x %02x\n", d5[3], d5[2], d5[1], d5[0]);
    printf("D7\t %02x %02x %02x %02x\n", d7[3], d7[2], d7[1], d7[0]);
}

