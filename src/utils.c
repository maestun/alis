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


int is_host_little_endian(void) {
    unsigned int x = 1;
    char * c = (char *)&x;
    return (int)*c;
}


// u32 swap32_old(u32 num, sPlatform pl) {

//     if (pl.is_little_endian != is_host_little_endian()) {
//         num = ((num >> 24) & 0xff) | // move byte 3 to byte 0
//                 ((num << 8) & 0xff0000) | // move byte 1 to byte 2
//                 ((num >> 8) & 0xff00) | // move byte 2 to byte 1
//                 ((num << 24) & 0xff000000); // byte 0 to byte 3
//     }
//     return num;
// }


// u16 swap16_old(u16 num, sPlatform pl) {

//     if(pl.is_little_endian != is_host_little_endian()) {
//         num = (num >> 8) | (num << 8);
//     }
//     return num;
// }


// u32 fread32_old(FILE * fp, sPlatform pl) {
//     u32 val = 0;
//     fread(&val, sizeof(u32), 1, fp);
//     return swap32_old(val, pl);
// }


// u16 fread16_old(FILE * fp, sPlatform pl) {
//     u16 val = 0;
//     fread(&val, sizeof(u16), 1, fp);
//     return swap16_old(val, pl);
// }


// u16 read16(const u8 *ptr, u8 is_le) {
    
//     return swap16(ptr, is_le);
// }


// u32 read24(const u8 *ptr, u8 is_le) {

//     return swap24(ptr, is_le);
// }


// u32 swap24(const u8 *value, u8 is_le) {
    
//     u32 result = 0;
//     memcpy((u8 *)&result, value, 3);

//     // byteswap
//     if (is_le != is_host_le())
//     {
//         result = (((result >> 24) & 0xff) | ((result <<  8) & 0xff0000) | ((result >>  8) & 0xff00)) >> 8;
//     }
    
//     // extend
//     if (result > 0x7FFFFF)
//     {
//         result = (result << 8) & 0xff;
//     }

//     return result;
// }


// u32 read32(const u8 *ptr, u8 is_le) {
    
//     return swap32(ptr, is_le);
// }

int is_host_le(void) {
    static unsigned int x = 1;
    char* c = (char*)&x;
    return (int)*c;
}

// u32 swap32(const u8 *value, u8 is_le) {
    
//     u32 result;
//     memcpy((u8 *)&result, value, sizeof(u32));
    
//     return is_le == is_host_le() ? result : ((result >> 24) & 0xff) |
//                                             ((result <<  8) & 0xff0000) |
//                                             ((result >>  8) & 0xff00) |
//                                             ((result << 24) & 0xff000000);
// }

u32 fread32(FILE* fp, u8 is_le) {
    u32 v = 0;
    fread(&v, sizeof(u32), 1, fp);
    return swap32((u8 *)&v);
}

// u16 swap16(const u8 *value, u8 is_le) {
    
//     u16 result;
//     memcpy((u8 *)&result, value, sizeof(u16));

//     return is_le == is_host_le() ? result : (result <<  8) |
//                                             (result >>  8);
// }

u16 fread16(FILE* fp, u8 is_le) {
    u16 v = 0;
    fread(&v, sizeof(u16), 1, fp);
    return swap16((u8 *)&v);
}
