//
//  utils.c
//  aodec
//
//  Created by zlot on 07/06/2018.
//  Copyright © 2018 zlot. All rights reserved.
//

#include "utils.h"


s16 sign_extend(s16 x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

s16 extend_w(s8 x) {
    return sign_extend(x, 8);
}

s32 extend_l(s16 val) {
    return val + (BIT_CHK(val, 7) ? 0xffff0000 : 0);
}

u32 reverse_bytes_32(s32 value) {
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 | (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

long get_file_size(char * file_name) {
    long sz = -1;
    FILE * fp = fopen(file_name, "r");
    if(fp) {
        fseek(fp, 0L, SEEK_END);
        sz = ftell(fp);
        fclose(fp);
    }
    return sz;
}

char * strlower(char * str) {
    char * ptr = str;
    if(ptr) {
        for ( ; *ptr; ++ptr)
            *ptr = tolower(*ptr);
    }
    return str;
}

char * strupper(char * str) {
    char * ptr = str;
    if(ptr) {
        for ( ; *ptr; ++ptr)
            *ptr = toupper(*ptr);
    }
    return str;
}


//u32 read_big_endian(u8 * data, size_t sz) {
//    u32 ret = 0;
//    u8 shift = sz - 1;
//    for(int i = 0; i < sz; i++) {
//        ret += (data[i]) << (shift-- << 3);
//    }
//    return ret;
//}


int is_host_little_endian() {
    unsigned int x = 1;
    char * c = (char *)&x;
    return (int)*c;
}


u32 swap32(u32 num, sPlatform pl) {

    if (pl.is_little_endian != is_host_little_endian()) {
        num = ((num >> 24) & 0xff) | // move byte 3 to byte 0
                ((num << 8) & 0xff0000) | // move byte 1 to byte 2
                ((num >> 8) & 0xff00) | // move byte 2 to byte 1
                ((num << 24) & 0xff000000); // byte 0 to byte 3
    }
    return num;
}


u16 swap16(u16 num, sPlatform pl) {

    if(pl.is_little_endian != is_host_little_endian()) {
        num = (num >> 8) | (num << 8);
    }
    return num;
}


u32 fread32(FILE * fp, sPlatform pl) {
    u32 val = 0;
    fread(&val, sizeof(u32), 1, fp);
    return swap32(val, pl);
}


u16 fread16(FILE * fp, sPlatform pl) {
    u16 val = 0;
    fread(&val, sizeof(u16), 1, fp);
    return swap16(val, pl);
}


u16 read16(const u8 *ptr, sPlatform pl) {
    
    return swap16(*(u16 *)ptr, pl);
}


u32 read32(const u8 *ptr, sPlatform pl) {
    
    return swap32(*(u32 *)ptr, pl);
}

