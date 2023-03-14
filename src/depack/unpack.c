//
//  unpack.c
//  silmarils-unpacker
//

#include "unpack.h"
#include "utils.h"

const u8 kPackerKindOld = 0x81;
const u8 kPackerKindOldInterlaced = 0x80;
const u8 kPackerKindNew = 0xa1;
const u8 kPackedHeaderSize = 6;
const u8 kPackedDictionarySize = 8;
const u8 kVMSpecsSize = 16;

#define BIT_CLR(v, b)       (v &= ~(1UL << b))
#define BIT_SET(v, b)       (v |= (1UL << b))
#define BIT_CHG(v, b)       (v ^= (1UL << b))
#define BIT_CHK(v, b)       ((v >> b) & 1U)


// ============================================================================
#pragma mark - Old Silmarils unpacker
// ============================================================================

s8 old_read_byte(u8** ptr_packed, u8* ptr_packed_end) {
    return *ptr_packed == ptr_packed_end ? 0 : *(*ptr_packed)++;
}

void old_write_byte(u8** ptr_unpacked, u8 byte, u8 inc) {
    **ptr_unpacked = byte;
    *ptr_unpacked += inc;
}

void unpack_old(u8* ptr_packed, const u32 packed_sz, 
                u8* ptr_unpacked, const u32 unpacked_sz,
                u8 is_interlaced) {

    u8* ptr_packed_end = ptr_packed + packed_sz;
    u8* ptr_unpacked_end = ptr_unpacked + unpacked_sz;

    const u8 inc = is_interlaced ? 8 : 1;
    u8 cnt = inc;

    do {
        while (ptr_unpacked < ptr_unpacked_end) {
            s8 d1 = old_read_byte(&ptr_packed, ptr_packed_end);
            s8 counter = d1;
            BIT_CLR(counter, 7);
            if (BIT_CHK(d1, 7)) {
                // read byte, and repeat copy
                d1 = old_read_byte(&ptr_packed, ptr_packed_end);
                while (counter--) {
                    old_write_byte(&ptr_unpacked, d1, inc);
                }
            }
            else {
                // copy bytes
                while (counter--) {
                    d1 = old_read_byte(&ptr_packed, ptr_packed_end);
                    old_write_byte(&ptr_unpacked, d1, inc);
                }
            }
        }
        // used only for interlaced
        ptr_unpacked -= unpacked_sz;
        ptr_unpacked++;
        ptr_packed++;
    }
    while (--cnt > 0);
}


// ============================================================================
#pragma mark - New Silmarils unpacker
// ============================================================================

/// @brief swap higher and lower words in long value
/// @param val value to alter
/// @return altered value
u32 swap(u32 val) {
   u32 hi = val & 0xffff0000;
   u32 lo = val & 0x0000ffff;
   return lo << 16 | hi >> 16;
}

/// @brief ROTate Left Long
/// @param shift number bits to rotate
/// @param value value to alter
/// @return altered value
u32 rotll(u8 shift, u32 value) {
    if ((shift &= sizeof(value)*8 - 1) == 0)
        return value;
    return (value << shift) | (value >> (sizeof(value)*8 - shift));
}

s8 byte(u32 value) {
    return value & 0xff;
}

s16 word(u32 value) {
    return value & 0xffff;
}

// TODO: determiner quel type de data on utilise pour les regs
s32 d5, d7;
u32 __unpack_counter = 0;


// USES 
// io d5
// io d7
void decode(u8** ptr_packed, u16 bit) {
    d5 &= 0xffff0000;
    
    d7 = (d7 & 0xffffff00) + ((byte(d7) - byte(bit)) & 0xff);
    if(byte(d7) < 0) {
        // worm
        // TODO: on add d7 car on a soustrait avant... 
        // autant comparer d0 et d7 dans le if au dessus
        // sans modifier d7 ?
        d7 = (d7 & 0xffffff00) + ((byte(d7) + byte(bit)) & 0xff);
        d5 = (d5 & 0xffff0000);
        d5 = rotll(d7, d5);
        d5 = swap(d5);

        // here, original code tests if we need more packed data chunks

        u8 hi = *(*ptr_packed)++;
        u8 lo = *(*ptr_packed)++;
        u16 pack_word = (hi << 8) + lo;
        d5 = (d5 & 0xffff0000) + pack_word;
        d5 = swap(d5);

        bit = ((byte(bit) - byte(d7)) & 0xff);
        d7 = 0x10;

        d5 = rotll(bit, d5);
        d7 = (d7 & 0xffffff00) + ((byte(d7) - byte(bit)) & 0xff);
    }
    else {
        d5 = rotll(bit, d5);
    }
}

void write_neg(u8** ptr_unpacked, s16 val, s16* counter) {
    // FML - neg lower word of d5

    s16 offset = val * -1;
    // SXX 
    do {
        u8 c = *(*ptr_unpacked + offset - 1);
        *(*ptr_unpacked)++ = c;
        __unpack_counter++;
    } while(--(*counter) != -1);
}

void count(u8** ptr_packed, u8 start, u8 stop, s16* counter) {
    do {
        decode(ptr_packed, start);
        *counter += word(d5);
    } while (word(d5) == stop);
}

/// @brief 
/// @param ptr_packed   pointer to raw packed data (w/o header, specs, dictionary)
/// @param ptr_unpacked pointer to allocated buffer for unpacked data
/// @param unpacked_sz  unpacked data size (read from packed header)
/// @param dictionary   dictionary (8-byte buffer read from packed header)
/// @return 
u32 unpack_new(u8* ptr_packed,
               u8* ptr_unpacked, 
               const u32 unpacked_sz,
               u8* dictionary) {

    s16 counter = 0;
    __unpack_counter = 0;
    u8* ptr_unpacked_end = ptr_unpacked + unpacked_sz - 1;
    d7 = 0;
    while(ptr_unpacked < ptr_unpacked_end) {
        // loop
        decode(&ptr_packed, 1);
        if(byte(d5) != 0) {
            counter = 0;
            
            // ZMB - get number of bytes to write to output
            count(&ptr_packed, 2, 3, &counter);

            // QQCH - write d2+1 unpacked bytes
            do {
                decode(&ptr_packed, 8);
                *(u8*)ptr_unpacked++ = byte(d5);
                __unpack_counter++;
            } while(--counter != -1);
            
            // check unpack end 
            // TODO: move test to MMI ?
            if(ptr_unpacked >= ptr_unpacked_end) {
                break;
            }
        }

        // MMI
        decode(&ptr_packed, 3);
        u16 bit = dictionary[byte(d5)];
        d5 = (d5 & 0xffff0000) + (word(d5 & 3));
        if(word(d5) != 0) {
            counter = word(d5);
            decode(&ptr_packed, bit);
            write_neg(&ptr_unpacked, word(d5), &counter);
        }
        else {
            // SNS
            decode(&ptr_packed, bit);
            u16 save_d5w = word(d5);
            counter = 0;

            // CRVL
            count(&ptr_packed, 3, 7, &counter);

            // CRVL2
            d5 = (d5 & 0xffff0000) + save_d5w;
            counter += 4;
            write_neg(&ptr_unpacked, word(d5), &counter);
        }
    }
    printf("unpack counter=%d\n", __unpack_counter);
    return __unpack_counter;
}


// ============================================================================
#pragma mark - Depacker
// ============================================================================

u8 is_packed(u8 packer_kind) {
    return packer_kind == kPackerKindOld || 
           packer_kind == kPackerKindOldInterlaced || 
           packer_kind == kPackerKindNew;
}

/// @brief Unpacks a script file to buffer
/// @param packed_file_path full path to packed file
/// @param is_le zero if file to unpack is big-endian encoded
/// @param unpacked_buffer output buffer
/// @return unpacked file size if success, a negative value if error, or
/// zero if the input file is not packed
int unpack_script(const char* packed_file_path,
                  u8 is_le,
                  u8** unpacked_buffer) {
    int ret = 0;
    FILE* pfp = fopen(packed_file_path, "rb");

    if(pfp) {
        // get packed sz
        fseek(pfp, 0, SEEK_END);
        u32 packed_size = ftell(pfp);
        rewind(pfp);

        u32 magic = fread32(pfp, is_le);
        u8 packer_kind = magic >> 24;

        if(is_packed(packer_kind)) {
            u16 is_main = fread16(pfp, is_le) == 0;
            u32 unpacked_size = (magic & 0x00ffffff);
            u8 dict[kPackedDictionarySize];

            packed_size -= kPackedHeaderSize; // size of magic + is_main
            unpacked_size -= kPackedHeaderSize; // TODO: not sure about that
            if(is_main) {
                // skip vm specs
                fseek(pfp, kVMSpecsSize, SEEK_CUR);
                packed_size -= kVMSpecsSize;
                unpacked_size -= kVMSpecsSize; // TODO: not sure about that
            }

            if(packer_kind == kPackerKindNew) {
                // read dictionary
                fread(dict, sizeof(u8), kPackedDictionarySize, pfp);
                packed_size -= kPackedDictionarySize;
            }

            // read packed bytes in alloc'd buffer
            u8* packed_buffer = (u8*)malloc(packed_size * sizeof(u8));
            fread(packed_buffer, sizeof(u8), packed_size, pfp);
            fclose(pfp);

            // alloc unpack buffer
            *unpacked_buffer = (u8*)malloc(unpacked_size * sizeof(u8));
            
// TODO: remove
// u8 ref[25328];
// FILE* f=fopen("/home/olivier/dev/self/unpack/data/ishar1/atari/MAIN.AO.ref", "rb");
// for(int i = 0; i < 25328; i++) {
//     ref[i]=fgetc(f);
// }
// fclose(f);

            // unpack
            if (packer_kind == kPackerKindNew) {
                unpack_new(packed_buffer, *unpacked_buffer, unpacked_size, dict);
                // depak(packed_buffer, unpacked_buffer, packed_size, unpacked_size, dict);
            }
            else if (packer_kind == kPackerKindOld) {
                unpack_old(packed_buffer, packed_size, *unpacked_buffer, unpacked_size, 0);
            }
            else if (packer_kind == kPackerKindOldInterlaced) {
                unpack_old(packed_buffer, packed_size, *unpacked_buffer, unpacked_size, 1);
            }
            printf("Unpacked %s: %d bytes into %d bytes (~%d%% packing ratio) using %s packer.\n", 
                    packed_file_path, packed_size, unpacked_size, 
                    100 - (int)((packed_size * 100) / unpacked_size),
                    packer_kind == kPackerKindNew ? "new" : "old");
            free(packed_buffer);
            packed_buffer = NULL;
            ret = unpacked_size;
        }
        else {
            // do nothing
            printf("%s is not packed, or packer format is unknown (0x%02x)\n", packed_file_path, (magic >> 24));
            ret = EUnpackErrorFormat;
        }
    }
    else {
        // error
        printf("Cannot open input file (%s)\n", packed_file_path);
        ret = EUnpackErrorInput;
    }
    return ret;
}
