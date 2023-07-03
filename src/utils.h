//
//  utils.h
//  aodec
//
//  Created by zlot on 07/06/2018.
//  Copyright © 2018 zlot. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include "config.h"
#include "platform.h"

long        get_file_size(char * file_name);
char *      strlower(char * str);
char *      strupper(char * str);
u32         reverse_bytes_32(s32 value);
s16         extend_w(s8 value);
s32         extend_l(s16 value);
//u32         read_big_endian(u8 * data, size_t sz);
u16         fread16_old(FILE * fp, sPlatform pl);
u32         fread32_old(FILE * fp, sPlatform pl);
u16         read16(const u8 *ptr, u8 is_le);
u32         read24(const u8 *ptr, u8 is_le);
u32         read32(const u8 *ptr, u8 is_le);
u16         swap16_old(u16 num, sPlatform pl);
u32         swap32_old(u32 num, sPlatform pl);

u16         fread16(FILE * fp, u8 is_le);
u32         fread32(FILE * fp, u8 is_le);
u16         swap16(const u8 *, u8 is_le);
u32         swap24(const u8 *, u8 is_le);
u32         swap32(const u8 *, u8 is_le);

int         is_host_le(void);

#endif /* utils_h */
