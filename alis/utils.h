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

long        get_file_size(char * file_name);
char *      strlower(char * str);
char *      strupper(char * str);
u32         reverse_bytes_32(s32 value);
s16         extend_w(s8 value);
s32         extend_l(s16 value);
u32         read_big_endian(u8 * data, size_t sz);

#endif /* utils_h */
