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

#include "alis.h"
#include "alis_private.h"
#include "debug.h"
#include "export.h"
#include "mem.h"
#include "screen.h"
#include "utils.h"
#include <stdlib.h>


void export_script(sAlisScriptData *script)
{
    s32 addr = script->data_org;
    s32 off = xread32(addr + 0xe);
    addr += off;
   
    // graphic resources
    s32 glen = xread16(addr + 0x4);
    for (int i = 0; i < glen; i++)
    {
        char name[32] = {0};
        strcpy(name, script->name);
        snprintf(strrchr(name, '.'), 16, "_%.3d", i);
       
        s32 rsrc = addr + xread32(addr) + i * 4;
        export_graphics(rsrc, name);
    }

    // audio resources
    s32 mlen = xread16(addr + 0x10);
    for (int i = 0; i < mlen; i++)
    {
        char name[32] = {0};
        strcpy(name, script->name);
        snprintf(strrchr(name, '.'), 16, "_%.3d", i);
       
        s32 at = xread32(addr + 0xc) + off + i * 4;
        s32 rsrc = xread32(script->data_org + at) + at;
        export_audio(script->data_org + rsrc, name);
    }

    // form resources
    addr += xread32(addr + 0x6);
   
    for (int i = 0; i < 16; i++)
    {
        u32 rsrc = addr + xread16(addr + (i * 2));

//        s16 x = 0;
//        s16 y = 0;
//        s16 z = 0;
//
//        u8 test = xread8(rsrc);
//        if (test == 0)
//        {
//            x = (s16)xread8(rsrc + 4) + ((s16)xread8(rsrc + 7) >> 1);
//            z = (s16)xread8(rsrc + 5) + ((s16)xread8(rsrc + 8) >> 1);
//            y = (s16)xread8(rsrc + 6) + ((s16)xread8(rsrc + 9) >> 1);
//        }
//        else if ((char)test < 0)
//        {
//            x = 0;
//            y = 0;
//            z = 0;
//        }
//        else
//        {
//            if (test == 1)
//            {
//                x = xread16(rsrc + 4) + (xread16(rsrc + 0xa) >> 1);
//                z = xread16(rsrc + 6) + (xread16(rsrc + 0xc) >> 1);
//                y = xread16(rsrc + 8) + (xread16(rsrc + 0xe) >> 1);
//            }
//            else
//            {
//                x = xread16(rsrc + 2);
//                z = xread16(rsrc + 4);
//                y = xread16(rsrc + 6);
//            }
//        }
//        
//        printf("\n%.6x: %.3d, %.3d, %.3d", rsrc, x, y, z);
    }
}

#pragma mark graphics

#pragma pack(push, 1)

typedef struct {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} BMPFileHeader;

typedef struct {
    u32 biSize;
    s32 biWidth;
    s32 biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    s32 biXPelsPerMeter;
    s32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} BMPInfoHeader;

typedef struct {
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} BMPColorTableEntry;

#pragma pack(pop)

void save_grayscale_bmp(const u8 *buffer, s32 width, s32 height, const char *filepath)
{
    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        debug(EDebugError, "Error: Unable to open file for writing.\n");
        return;
    }

    int rowSize = (width + 3) & (~3);
    int padding = rowSize - width;

    BMPFileHeader fileHeader;
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 256 * sizeof(BMPColorTableEntry);
    fileHeader.bfSize = fileHeader.bfOffBits + rowSize * height;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;

    BMPInfoHeader infoHeader;
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = -height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 8;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = rowSize * height;
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;
    infoHeader.biClrUsed = 256;
    infoHeader.biClrImportant = 256;

    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    BMPColorTableEntry colorTable[256];
    for (int i = 0; i < 256; i++)
    {
        colorTable[i].rgbBlue = i;
        colorTable[i].rgbGreen = i;
        colorTable[i].rgbRed = i;
        colorTable[i].rgbReserved = 0;
    }
    
    fwrite(colorTable, sizeof(BMPColorTableEntry), 256, file);

    for (int y = 0; y < height; y++)
    {
        fwrite(buffer + y * width, sizeof(u8), width, file);
        
        for (int p = 0; p < padding; p++)
            fputc(0, file);
    }

    fclose(file);
}

void save_palette(u8 *resourcedata, const char *filepath)
{
    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        debug(EDebugError, "Error: Unable to open file for writing.\n");
        return;
    }

    u8 palette[768];
    memset(palette, 0, 768);

    // palette
    
    if (alis.platform.kind == EPlatformMac)
    {
        palette[0] = 0xff;
        palette[1] = 0xff;
        palette[2] = 0xff;
    }
    else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
    {
        memcpy(palette, cga_palette, 12);
    }
    else
    {
        s16 colors = resourcedata[1];
        if (colors == 0) // 4 bit palette
        {
            image.palc = 0;
            u8 *palptr = &resourcedata[2];
            
            s16 to = 0;
            if (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAmigaAGA)
            {
                for (s32 i = 0; i < 16; i++)
                {
                    palette[to++] = (palptr[i * 2 + 0] & 0b00001111) << 4;
                    palette[to++] = (palptr[i * 2 + 1] >> 4) << 4;
                    palette[to++] = (palptr[i * 2 + 1] & 0b00001111) << 4;
                }
            }
            else
            {
                for (s32 i = 0; i < 16; i++)
                {
                    palette[to++] = (palptr[i * 2 + 0] & 0b00000111) << 5;
                    palette[to++] = (palptr[i * 2 + 1] >> 4) << 5;
                    palette[to++] = (palptr[i * 2 + 1] & 0b00000111) << 5;
                }
            }
        }
        else // 8 bit palette
        {
            if (alis.platform.kind == EPlatformAmiga)
            {
                image.palc = 0;
                u8 *palptr = &resourcedata[2];
                
                s16 to = 0;
                
                for (s32 i = 0; i < 32; i++)
                {
                    palette[to++] = (palptr[i * 2 + 0] & 0b00001111) << 4;
                    palette[to++] = (palptr[i * 2 + 1] >> 4) << 4;
                    palette[to++] = (palptr[i * 2 + 1] & 0b00001111) << 4;
                }
            }
            else
            {
                u8 offset = resourcedata[2];
                memcpy(palette + (offset * 3), resourcedata + 4, (1 + colors) * 3);
            }
        }
    }
    
    fwrite(palette, 1, 768, file);
    fclose(file);
}

void export_graphics(u32 addr, const char *name)
{
    u8 *resourcedata = alis.mem + addr + xread32(addr);
    if (resourcedata[0] > 0x80)
    {
        if (resourcedata[0] == 0xfe)
        {
            char path[kPathMaxLen] = {0};
            strcpy(path, alis.platform.path);
            strcat(path, name);
            strcat(path, ".act");

            save_palette(resourcedata, path);
        }
        else
        {
            // composit images
            
//            s16 rsrccount = resourcedata[1];
//            u8 *currsrc = resourcedata + 2;
//            
//            u8 invx;
//            
//            for (s32 i = 0; i < rsrccount; i++)
//            {
//                invx = 0;
//                
//                s16 curelem = read16(currsrc + 0);
//                s16 curdepx = read16(currsrc + 2);
//                if (invx != 0)
//                    curdepx = -curdepx;
//                
//                s16 curdepy = read16(currsrc + 4);
//                s16 curdepz = read16(currsrc + 6);
//                
//                if (curelem < 0)
//                {
//                    curelem = curelem & 0x7fff;
//                    invx ^= 1;
//                }
//                
//                // TODO: add to composite
//                
//                currsrc += 8;
//            }
        }
    }
    else
    {
        u8 type = resourcedata[0];
        s16 width = read16(resourcedata + 2) + 1;
        s16 height = read16(resourcedata + 4) + 1;

        u8 *bmpdata = resourcedata + 6;
        u8 *tgt = NULL;

        switch (type)
        {
            case 0x01:
            {
                // NOP
                break;
            }
                
            case 0x14:
            case 0x16:
            {
                tgt = bmpdata;
                break;
            }

            case 0x00:
            case 0x02:
            {
                tgt = malloc(width * height);
                if (alis.platform.kind == EPlatformMac)
                {
                    convert_mac_mono(tgt, bmpdata, width, height);
                }
                else if (alis.platform.kind == EPlatformPC && alis.platform.uid == EGameMadShow)
                {
                    convert_cga(tgt, bmpdata, width, height);
                }
                else
                {
                    convert_4bit_st(tgt, bmpdata, width, height);
                }

                break;
            }
                
            case 0x10:
            case 0x12:
            {
                tgt = malloc(width * height);
                if (alis.platform.px_format == EPxFormatAmPlanar)
                {
                    convert_5bit_ami(tgt, bmpdata, width, height);;
                }
                else
                {
                    convert_4bit(tgt, bmpdata, width, height);
                }
                break;
            }
                
            case 0x40:
            {
                // TODO: video
                break;
            }
                
            case 0x7f:
            {
                // TODO: map
                break;
            }
        }
        
        if (tgt)
        {
            char path[kPathMaxLen] = {0};
            strcpy(path, alis.platform.path);
            strcat(path, name);
            strcat(path, ".bmp");

            save_grayscale_bmp(tgt, width, height, path);
            free(tgt);
        }
    }
}

void convert_mac_mono(u8 *tgt, u8 *bmpdata, s16 width, s16 height)
{
    u8 index, color;
    
    for (s32 h = 0; h < height; h++)
    {
        for (s32 w = 0; w < width; w++, tgt++)
        {
            index = w % 4;
            color = *(bmpdata + (w / 4) + h * (width / 4));
            color = (color & masks[index]) >> rots[index];
            *tgt = !(color & 2) ? !(color & 1) * 0xff : 0x7f;
        }
    }
}

void convert_cga(u8 *tgt, u8 *bmpdata, s16 width, s16 height)
{
    u8 index, color;
    
    for (s32 h = 0; h < height; h++)
    {
        for (s32 w = 0; w < width; w++, tgt++)
        {
            index = w % 4;
            color = *(bmpdata + (w / 4) + h * (width / 4));
            color = (color & masks[index]) >> rots[index];
            *tgt = color * 64;
        }
    }
}

void convert_4bit_st(u8 *tgt, u8 *bmpdata, s16 width, s16 height)
{
    u8 color;

    for (s32 h = 0; h < height; h++)
    {
        for (s32 w = 0; w < width; w++, tgt++)
        {
            color = *(bmpdata + (w / 2) + h * (width / 2));
            *tgt = w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
        }
    }
}

void convert_4bit(u8 *tgt, u8 *bmpdata, s16 width, s16 height)
{
    u8 color;
    u8 palidx = bmpdata[0];
    bmpdata += 2;
    
    for (s32 h = 0; h < height; h++)
    {
        for (s32 w = 0; w < width; w++, tgt++)
        {
            color = *(bmpdata + (w / 2) + h * (width / 2));
            *tgt = palidx + w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
        }
    }
}

void convert_5bit_ami(u8 *tgt, u8 *bmpdata, s16 width, s16 height)
{
    u32 planesize = (width * height) / 8;
    u8 c0, c1, c2, c3, c4;
    
    for (s32 h = 0; h < height; h++)
    {
        for (s32 w = 0; w < width; w++, tgt++)
        {
            s32 idx = (w + h * width) / 8;
            c0 = *(bmpdata + idx);
            c1 = *(bmpdata + (idx += planesize));
            c2 = *(bmpdata + (idx += planesize));
            c3 = *(bmpdata + (idx += planesize));
            c4 = *(bmpdata + (idx += planesize));
            
            int bit = 7 - (w % 8);
            
            *tgt = ((c0 >> bit) & 1) | ((c1 >> bit) & 1) << 1 | ((c2 >> bit) & 1) << 2 | ((c3 >> bit) & 1) << 3 | ((c4 >> bit) & 1) << 4;
        }
    }
}

void save_map(u8 *tgtdata, sSprite *sprite, u32 mapaddr)
{
    s32 vram = xread32(xread16(mapaddr - 0x24) + alis.atent);
    if (vram != 0)
    {
        u16 tileidx;
        u16 tileoffset = xread16(mapaddr + 0x24);
        u16 tileadd = xread16(mapaddr - 0x22);
        u16 tilecount = xread16(mapaddr - 0x20);
        u16 tilex = xread16(mapaddr - 0x1c);
        u16 tiley = xread16(mapaddr - 0x18);
        
        u16 testa = xread16(mapaddr - 6);
        u16 testb = xread16(mapaddr - 10);
        
        // TODO: ...
        return;

        s32 addr = get_0x14_script_org_offset(vram);
        addr = xread32(addr + 0xe) + addr;
        addr = xread32(addr) + addr;

        u16 tempy = (image.blocy1 - sprite->newy) + xread16(mapaddr - 6);
        s32 yc = tempy / (u16)tiley;
        s16 yo = image.blocy1 - tempy % (u16)tiley;

        u32 tileaddr = mapaddr + 0x28;
        s16 t1 = image.blocx1 - sprite->newx;
        s16 t2 = xread16(mapaddr - 10);
        
        tileaddr += (tileoffset * ((u16)(t1 + t2) / tilex) + yc);

        s32 pxmapw = image.blocx2 - image.blocx1;
        s32 pxmaph = image.blocy2 - image.blocy1;

        s16 mapheight = ((image.blocy2 - sprite->newy) + xread16(mapaddr - 6)) / (u16)tiley - yc;
        s16 mapwidth = (image.blocx2 - image.blocx1) / tilex;

        if ((xread8(mapaddr - 0x26) & 2) != 0)
        {
            yo += ((u16)(tiley - 1) >> 1);
            if (yc != 0)
            {
                mapheight ++;
                tileaddr --;
                yo -= tiley;
            }
            
            mapheight ++;
        }
        
        s32 addy = alis.platform.kind == EPlatformPC ? 0 : 1;

        if (alis.platform.bpp == 8)
        {
            memset(tgtdata, 63, pxmapw * pxmaph);
        }

        for (int mh = mapheight; mh >= 0; mh--)
        {
            for (int mw = mapwidth; mw >= 0; mw--)
            {
                if (xread8(tileaddr) != 0 && (tileidx = (xread8(tileaddr) + tileadd) - 1) <= tilecount)
                {
                    vram = addr + (s16)(tileidx * 4);
                    u32 img = xread32(vram) + vram;
                    u8 *bitmap = (alis.mem + img);
                    s16 width = (s16)read16(bitmap + 2) + 1;
                    s16 height = (s16)read16(bitmap + 4) + 1;

                    u8 *at = bitmap + 6;

                    s16 posx1 = (mapwidth - mw) * tilex;
                    s16 posy1 = ((mapheight - mh) - addy) * tiley + (((tiley - 1) - height) / 2);

                    u8 flip = 0;
                    u8 color = 0;
                    u8 clear = 0;
                    u8 palidx = 0;

                    s16 bmpx1 = 0;
                    s16 bmpx2 = width;
                    s16 bmpy1 = 0;
                    s16 bmpy2 = height;
                    
                    switch (bitmap[0])
                    {
                        case 0x01:
                        {
                            // rectangle
                            
                            color = bitmap[1];
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = tgtdata + (bmpx1 + posx1) + ((posy1 + h) * pxmapw);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    *tgt = color;
                                }
                            }
                            break;
                        }
                            
                        case 0x00:
                        case 0x02:
                        {
                            // ST image
                            
                            clear = bitmap[0] == 0 ? 0 : -1;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = tgtdata + (bmpx1 + posx1) + ((posy1 + h) * pxmapw);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        *tgt = color;
                                    }
                                }
                            }

                            break;
                        }
                            
                        case 0x10:
                        case 0x12:
                        {
                            // 4 bit image
                            
                            palidx = bitmap[6];
                            clear = bitmap[0] == 0x10 ? bitmap[7] : -1;
                            
                            at = bitmap + 8;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = tgtdata + (bmpx1 + posx1) + ((posy1 + h) * pxmapw);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        *tgt = palidx + color;
                                    }
                                }
                            }
                            
                            break;
                        }
                            
                        case 0x14:
                        case 0x16:
                        {
                            // 8 bit image
                            
                            palidx = bitmap[6]; // NOTE: not realy sure what it is, but definetly not palette index
                            clear = bitmap[0] == 0x14 ? bitmap[7] : -1;
                            
                            at = bitmap + 8;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = tgtdata + (bmpx1 + posx1) + ((posy1 + h) * pxmapw);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    color = *(at + (flip ? width - (w + 1) : w) + h * width);
                                    if (color != clear)
                                    {
                                        *tgt = color + palidx;
                                    }
                                }
                            }
                            
                            break;
                        }
                    };
                }
                
                tileaddr += tileoffset;
            }

            tileaddr += (s16)(1 - ((s16)mapwidth + 1) * tileoffset);
        }
    }
}

#pragma mark audio

#pragma pack(push, 1)

typedef struct {
    u8  chunkID[4];
    u32 chunkSize;
    u8  format[4];
} WAVRIFFHeader;

typedef struct {
    u8  subchunk1ID[4];
    u32 subchunk1Size;
    u16 audioFormat;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
} WAVFmtHeader;

typedef struct {
    u8  subchunk2ID[4];
    u32 subchunk2Size;
} WAVDataHeader;

#pragma pack(pop)

void save_audio_wav(const u8 *buffer, u32 length, u32 sampleRate, const char *filepath)
{
    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        debug(EDebugError, "Error: Unable to open file for writing.\n");
        return;
    }

    WAVRIFFHeader riffHeader = {
        {'R', 'I', 'F', 'F'},
        36 + length,
        {'W', 'A', 'V', 'E'}
    };

    WAVFmtHeader fmtHeader = {
        {'f', 'm', 't', ' '},
        16,
        1,
        1,
        sampleRate,
        sampleRate,
        1,
        8
    };

    WAVDataHeader dataHeader = {
        {'d', 'a', 't', 'a'},
        length
    };
    
    u8 *swapped = malloc(length);
    for (int i = 0; i < length; i++)
    {
        swapped[i] = buffer[i] + 0x80;
    }

    fwrite(&riffHeader, sizeof(WAVRIFFHeader), 1, file);
    fwrite(&fmtHeader, sizeof(WAVFmtHeader), 1, file);
    fwrite(&dataHeader, sizeof(WAVDataHeader), 1, file);
    fwrite(swapped, sizeof(u8), length, file);
    fclose(file);

    free(swapped);
}

void export_audio(u32 addr, const char *name)
{
    s8 type = xread8(addr);
    if (type == 1 || type == 2)
    {
        u32 freq = xread8(addr + 1);
        u32 length = ((alis.platform.kind == EPlatformPC && (alis.platform.uid == EGameColorado || alis.platform.uid == EGameWindsurfWilly || alis.platform.uid == EGameMadShow || alis.platform.uid == EGameLeFeticheMaya)) ? xread32be(addr + 2) : xread32(addr + 2)) - 0x10;
        
        if (freq < 0x1 || 0x14 < freq)
            freq = 10;

        freq *= 1000;

        u8 *buffer = alis.mem + addr + 0x10;

        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        strcat(path, name);
        strcat(path, ".wav");
        
        save_audio_wav(buffer, length, freq, path);
    }
    else if (type == 0 || type == 3 || type == 4)
    {
        // music
    }
    else if (type == 5 || type == 6)
    {
        // chipmusic instruments
    }
}

// Export terrain mesh as Wavefront OBJ format
// Extracts terrain grid data to create a 3D mesh
// DATA FORMAT: Each grid cell stores TWO signed 16-bit height values in 4 bytes:
//   - Offset +0: s16 height value (signed, already in world units)
//   - Offset +2: s16 height value (signed, already in world units)
// Helper function to convert 9-bit palette color (3-bit RGB components 0-7) to 32-bit RGBA
static inline u32 palette_to_rgba(u16 pal_index, u16 palette[16][3])
{
    // Palette stores 3-bit values (0-7) for each component
    // Convert to 8-bit (0-255) by scaling: value * 255 / 7
    u8 r = (u8)((palette[pal_index][0] * 255) / 7);
    u8 g = (u8)((palette[pal_index][1] * 255) / 7);
    u8 b = (u8)((palette[pal_index][2] * 255) / 7);
    u8 a = 255;  // Full alpha

    return (a << 24) | (b << 16) | (g << 8) | r;
}

// The height values are stored DIRECTLY - no altitude table lookup needed!
// This allows the terrain to be rendered as a simple triangulated mesh
void export_terrain_obj(s32 scene_addr, s32 render_context, const char *filepath)
{
    u16 palette[16][3] = {{ 0, 0, 0 }, { 0, 0, 0 }, { 3, 3, 3 }, { 4, 4, 4 }, { 4, 4, 4 }, { 5, 5, 5 }, { 2, 3, 4 }, { 5, 5, 4 }, { 2, 2, 0 }, { 2, 3, 0 }, { 3, 4, 0 }, { 2, 2, 1 }, { 3, 2, 2 }, { 4, 3, 2 }, { 5, 4, 3 }, { 6, 5, 4 }};
    
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing\n", filepath);
        return;
    }

    // Get terrain dimensions from scene data
    u16 grid_width = xread16(scene_addr + 0x12) + 1;
    u16 grid_height = xread16(scene_addr + 0x14) + 1;

    // Load terrain data pointers
    s32 terrain_data = image.atlland;  // Terrain height map grid (4 bytes per cell)
    s32 altitude_table = image.atalti; // Altitude lookup table (s16 values)

    // Scale factors for world-space coordinates
    float world_scale_x = 1.0f;
    float world_scale_y = -0.25f;
    float world_scale_z = 1.0f;

    printf("Exporting terrain: %u x %u grid to %s\n", grid_width, grid_height, filepath);

    // Write OBJ header
    fprintf(file, "# Terrain mesh exported from ALIS Robinson's Requiem\n");
    fprintf(file, "# Grid dimensions: %u x %u cells\n", grid_width, grid_height);
    fprintf(file, "# Terrain data format (4 bytes per cell):\n");
    fprintf(file, "#   Bytes 0-1 (s16): terrain_type (upper 8 bits) + altitude_index (lower 8 bits)\n");
    fprintf(file, "#   Bytes 2-3 (s16): terrain_type (upper 8 bits) + altitude_index (lower 8 bits)\n");
    fprintf(file, "# Height lookup: altitude_table[altitude_index * 2]\n");
    fprintf(file, "# Vertices: 2 per cell (256x125 grid = 64000 vertices)\n");
    fprintf(file, "\n");

    // Export vertices
    // Each grid cell (x,y) contains 4 bytes (2 x s16 values)
    // Lower byte of each s16 = altitude table index, upper byte = terrain type
    //
    // NOTE: Terrain supports overhangs/bridges (3D perspective feature)
    // tbarland() renders overhangs when height[Y] < height[Y+1] under specific 3D conditions
    // However, in a flat mesh export (no perspective), overhangs need different handling
    // Current approach: simple height-based mesh without overhang geometry
    // (prevents artifacts from naive height difference detection)
    u32 vertex_count = 0;

    s16 vdarkw = xread8(render_context - 0x255) << 8; // 0x0600
    
    for (u16 y = 0; y < grid_height; y++)
    {
        for (u16 x = 0; x < grid_width; x++)
        {
            // Get altitude base pointer for this X column
            s32 cell_ptr = terrain_data + (x * 4);
            u32 altitude_base = xread32(cell_ptr);
            u32 cellbase = altitude_base + (y * 2);

            // Read 4 bytes at Y position as 2 x s16 values
            // s16 contains: upper byte = terrain type, lower byte = altitude index
            s16 cell0 = xread16(cellbase + 0);
            s16 cell1 = xread16(cellbase + 2);

            // Extract altitude indices from lower bytes
            u8 alt_index0 = (u8)(cell0 & 0xff);
            u8 alt_index1 = (u8)(cell1 & 0xff);

            // Look up heights in altitude table
            s16 height0 = xread16(altitude_table + (alt_index0 * 2));
            s16 height1 = xread16(altitude_table + (alt_index1 * 2));
            
            s16 index = ((cell0 & 0x3f00) >> 3) - 0xc00;
            
//            u16 a = xread16(render_context - 0x3c4); // 0x0100
//            u8 b = xread8(cellbase);           // 0xc0
//            u8 b1 = b & 0xc0;                       // 0xc0
//            u8 c = xread8(cellbase + 2);       // 0x40
//            u8 c1 = c & 0xc0;                       // 0x40
//            u16 d = xread16(a + cellbase);     // 0xc000
//            u16 d1 = d & 0xc0;                      // 0x0

            u16 pixels = vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + cellbase) & 0xc0) + (xread8(cellbase + 2) & 0xc0) + (xread8(cellbase) & 0xc0) * 2) * -2;

//            u16 pixels = 0x0200; // vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + altitude_base) & 0xc0) + (xread8(altitude_base + 2) & 0xc0) + (xread8(altitude_base) & 0xc0) * 2) * -2;
            u32 color = (u32)pixels;
            if ((s32)(color << 0x10) < 0)
            {
                color = 0;
            }
            
            color = (u16)xread16(alis.ptrdark + (s16)((color & 0x0000ff00) | ((xread8(render_context + 8 + index) * 2) & 0x000000ff)));
            
//            printf("\n[%.4x][%.8x][%.4x][%.4x][%.2x][%.2x][%.2x][%.2x][%.4x][%.4x] = [%.4x] = [%.8x]", (u16)index, cellbase, (u16)vdarkw, a, b, b1, c, c1, d, d1, pixels, color);

            // Extract two palette indices from color variable (first and second bits)
            // Create dithering by averaging two palette colors
            u16 pal_idx1 = (color & 0x0f);           // First palette index (lower 4 bits)
            u16 pal_idx2 = ((color >> 4) & 0x0f);    // Second palette index (next 4 bits)

            // Clamp positive heights to 0 (terrain doesn't go above ground)
            if (height0 > 0) height0 = 0;
            if (height1 > 0) height1 = 0;

            // Create first vertex at (x, height0, y) with averaged color
            float pos_x1 = (float)(grid_width - (x + 1.0f)) * world_scale_x;
            float pos_y1 = (float)height0 * world_scale_y;
            float pos_z1 = (float)y * world_scale_z;

            // Convert averaged color to 0-1 range for OBJ format
            float r_norm = ((palette[pal_idx1][0] / 7.0) + (palette[pal_idx2][0] / 7.0)) / 2.0;
            float g_norm = ((palette[pal_idx1][1] / 7.0) + (palette[pal_idx2][1] / 7.0)) / 2.0;
            float b_norm = ((palette[pal_idx1][2] / 7.0) + (palette[pal_idx2][2] / 7.0)) / 2.0;

            fprintf(file, "v %.6f %.6f %.6f %.6f %.6f %.6f\n", pos_x1, pos_y1, pos_z1, r_norm, g_norm, b_norm);
            vertex_count++;

            // Create second vertex at (x+0.5, height1, y) with averaged color
            float pos_x2 = (float)(grid_width - (x + 1.5f)) * world_scale_x;
            float pos_y2 = (float)height1 * world_scale_y;
            float pos_z2 = (float)y * world_scale_z;
            fprintf(file, "v %.6f %.6f %.6f %.6f %.6f %.6f\n", pos_x2, pos_y2, pos_z2, r_norm, g_norm, b_norm);
            vertex_count++;
        }
    }

    fprintf(file, "\n");

    // Export face indices (triangles)
    // Connect adjacent vertices to form terrain surface
    u32 face_count = 0;

    for (u16 y = 0; y < grid_height - 1; y++)
    {
        for (u16 x = 0; x < grid_width - 1; x++)
        {
            // Each cell (x,y) contributes 2 vertices (using just the first for now)
            u32 v0_a = (y * grid_width + x) * 2 + 1;           // (x,y) vertex 1
            u32 v1_a = (y * grid_width + (x + 1)) * 2 + 1;     // (x+1,y) vertex 1
            u32 v2_a = ((y + 1) * grid_width + x) * 2 + 1;     // (x,y+1) vertex 1
            u32 v3_a = ((y + 1) * grid_width + (x + 1)) * 2 + 1;  // (x+1,y+1) vertex 1

            // Create triangulated surface
            fprintf(file, "f %u %u %u\n", v2_a, v1_a, v0_a);
            face_count++;

            fprintf(file, "f %u %u %u\n", v2_a, v3_a, v1_a);
            face_count++;
        }
    }

    fclose(file);

    printf("Export complete: %u vertices, %u faces\n", vertex_count, face_count);
}

// Debug export: dumps raw terrain and altitude data for analysis
// This helps understand the exact memory layout and indexing
// Export overhang/bridge geometry as separate 3D object
// IMPORTANT: This implementation is incomplete - needs to account for:
// 1. Terrain segment offsets (render_context + 0x10 + index)
// 2. Proper grid iteration matching doland() perspective projection
// 3. fbottom flag conditions for valid overhang detection
//
// Current simple height difference detection doesn't capture the actual
// perspective projection logic used in tbarland().
void export_terrain_overhangs(s32 scene_addr, s32 render_context, const char *filepath)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing\n", filepath);
        return;
    }

    // Get terrain dimensions from scene data
    u16 grid_width = xread16(scene_addr + 0x12) + 1;
    u16 grid_height = xread16(scene_addr + 0x14) + 1;

    // Load terrain data pointers
    s32 terrain_data = image.atlland;
    s32 altitude_table = image.atalti;

    printf("Exporting overhang candidates: %u x %u grid to %s\n", grid_width, grid_height, filepath);

    // Write OBJ header
    fprintf(file, "# Overhang/Bridge geometry - EXPERIMENTAL\n");
    fprintf(file, "# WARNING: This export uses simplified overhang detection\n");
    fprintf(file, "# Actual overhangs in game require perspective projection calculation\n");
    fprintf(file, "# Grid dimensions: %u x %u cells\n", grid_width, grid_height);
    fprintf(file, "\n");

    u32 vertex_count = 0;
    u32 overhang_count = 0;

    // Scale factors for world-space coordinates
    float world_scale_x = 1.0f;
    float world_scale_y = -0.25f;
    float world_scale_z = 1.0f;

    // Scan for cells where height changes between Y and Y+1
    // NOTE: This is a simplified heuristic, not the actual fbottom calculation
    for (s32 y = 0; y < grid_height - 1; y++)
    {
        for (s32 x = 0; x < grid_width; x++)
        {
            // Get altitude base pointer for this X column
            s32 cell_ptr = terrain_data + (x * 4);
            u32 altitude_base = xread32(cell_ptr);
            u32 cellbase = altitude_base + (y * 2);
            
            // Read altitude indices at Y and Y+1
            // WARNING: doland() uses render_context + 0x10 + index for terrain segment offset
            // This simple approach uses direct altitude_base without segment offset
            s16 cell0 = xread16(cellbase + 0);
            s16 cell1 = xread16(cellbase + 2);
            
            u8 alt_index0 = (u8)(cell0 & 0xff);
            u8 alt_index1 = (u8)(cell1 & 0xff);
            
            s16 index0 = ((cell0 & 0x3f00) >> 3) - 0xc00;
            // s16 index1 = ((cell1 & 0x3f00) >> 3) - 0xc00;
            
            s32 terrain_cell = xread32(render_context + 0x10 + index0) + (2 * y) + xread32(terrain_data + (x << 2));
            if (terrain_cell == 0x001f6c8e)
            {
                sleep(0);
            }
            
            if (altitude_base == 0x001f6c8e)
            {
                sleep(0);
            }
            
            if (altitude_base == terrain_cell)
            {
                continue;
            }
            
            // contains barsprite?
            if ((s8)xread8(render_context + index0 + 0x14) == 1)
            {
//                u8 zoom_lvl = xread8(render_context + 0x15 + index0);
                u32 bitmap_ptr = xread32(render_context + 0x10 + index0);
                u32 bitmap = xread32(bitmap_ptr) + bitmap_ptr;
                if ((s8)xread8(bitmap) == 3)
                {
                    bitmap_ptr += xread16(bitmap + 2) * 4;
                    bitmap = xread32(bitmap_ptr) + bitmap_ptr;
                }

                s32 bitmap_height = xread16(bitmap + 4) + 1;
                s32 bitmap_width = xread16(bitmap + 2) + 1;
                
                u8 *bitmap_data = alis.mem + bitmap + 4;
                
//                s32 sprite_height = (bitmap_height >> (zoom_lvl & 0x3f)) - 1;
//                s32 sprite_width = (bitmap_width >> (zoom_lvl & 0x3f)) - 1;
                
                // TODO: add bilboard object using bitmap

                sleep(0);
            }

            // Look up heights
            s16 h0 = xread16(altitude_table + (alt_index0 * 2));
            s16 h1 = xread16(altitude_table + (alt_index1 * 2));

            s16 height0 = xread16(terrain_cell + (alt_index0 * 2));
            s16 height1 = xread16(terrain_cell + (alt_index1 * 2));
            
            s16 terrain_data = xread16(terrain_cell);
            s16 toph = terrain_data & 0xff;
            s16 index3 = ((terrain_data & 0x3f00) >> 3) - 0xc00;
            s16 test = toph - alt_index0;
//            if ((test != 0 && sborrow2(toph, alt_index_y) == (s32)((u32)test << 0x10) < 0) && ((position_y = xread16(render_context + 0x16 + index) - test) != 0 && sborrow2(xread16(render_context + 0x16 + index), test) == (s32)((u32)position_y << 0x10) < 0))
            if (test == 0)
                continue;
            
            // -------------------
            s16 fbottom = (s16)(alt_index0 + y) < toph;
            if ((bool)fbottom)
            {
                terrain_data = (alt_index0 + y) - alt_index0;
            }
            
            s32 prectopi = 100;
            s32 precboti = -100;

            s16 sVar3 = terrain_data - toph;
            terrain_data = xread16(render_context - 0x276) + xread16(altitude_table + (s16)(toph * 2));
            s16 screen_y = xread16(render_context - 0x246);
            image.toppixy = terrain_data;
//            xwrite16(render_context - 0x246, position_x);
            s32 position_y = prectopi;
            prectopi = terrain_data;
            if ((s16)terrain_data < (s16)position_y)
            {
//                xwrite16(render_context - 0x246, prectopi);
                //                            prectopi = altitude_index;
                position_y = prectopi;
                terrain_data = prectopi;
            }
            
            prectopi = terrain_data;
            if (fbottom != 0)
            {
                screen_y = xread16(render_context - 0x276) + xread16(altitude_table + (s16)(sVar3 * -2));
            }
            
            s32 botalt = precboti;
            s32 bothigh = screen_y;
            if (precboti < screen_y)
            {
                bothigh = precboti;
                botalt = screen_y;
            }
            
            precboti = screen_y;
//            if (fprectop != 0)
            {
                terrain_data = botalt - position_y;
//                d1 = position_x;
                if (terrain_data != 0)
                {
                    sleep(0);
                }
                else
                {
                    continue;
                }
            }
            
            if (prectopi >= 20000)
                prectopi -= 20000;
// -------------------
            
            // Clamp positive heights to 0
            if (height0 > 0) height0 = 0;
            if (height1 > 0) height1 = 0;

            // Check if there's a significant height difference (potential overhang)
            s16 height_diff = height1 - height0;

            // Overhang when next cell is higher than current
            // Minimum threshold of 1 unit to filter noise
//            if (height_diff > 1)
            //if (height0 != 0)
            {
                // Create box geometry for the overhang
                // Lower surface at height_y, upper surface at height_y_next
                float x_min = (float)(grid_width - (x + 0.0f)) * world_scale_x;
                float x_max = (float)(grid_width - (x + 1.0f)) * world_scale_x;
                float z_min = (float)(y + 0.0f) + world_scale_z;
                float z_max = (float)(y + 1.0f) + world_scale_z;
                float y_bottom = precboti * world_scale_y; // (float)height_y * 0.5f;
                float y_top = prectopi * world_scale_y; //(float)height_y_next * 0.5f;

                // Create 8 vertices for the overhang box
                                
                fprintf(file, "v %.6f %.6f %.6f\n", x_min, y_bottom, z_min);  // v0
                fprintf(file, "v %.6f %.6f %.6f\n", x_max, y_bottom, z_min);  // v1
                fprintf(file, "v %.6f %.6f %.6f\n", x_max, y_bottom, z_max);  // v2
                fprintf(file, "v %.6f %.6f %.6f\n", x_min, y_bottom, z_max);  // v3

                fprintf(file, "v %.6f %.6f %.6f\n", x_min, y_top, z_min);     // v4
                fprintf(file, "v %.6f %.6f %.6f\n", x_max, y_top, z_min);     // v5
                fprintf(file, "v %.6f %.6f %.6f\n", x_max, y_top, z_max);     // v6
                fprintf(file, "v %.6f %.6f %.6f\n", x_min, y_top, z_max);     // v7

                vertex_count += 8;

                // Create faces for the box
                u32 base = vertex_count - 8 + 1;  // OBJ uses 1-based indexing

                // Bottom face
                fprintf(file, "f %u %u %u\n", base, base+1, base+2);
                fprintf(file, "f %u %u %u\n", base, base+2, base+3);

                // Top face
                fprintf(file, "f %u %u %u\n", base+4, base+6, base+5);
                fprintf(file, "f %u %u %u\n", base+4, base+7, base+6);

                // Side faces
                fprintf(file, "f %u %u %u\n", base, base+4, base+5);  // Front
                fprintf(file, "f %u %u %u\n", base, base+5, base+1);
                fprintf(file, "f %u %u %u\n", base+2, base+6, base+7);  // Back
                fprintf(file, "f %u %u %u\n", base+2, base+7, base+3);
                fprintf(file, "f %u %u %u\n", base+3, base+7, base+4);  // Left
                fprintf(file, "f %u %u %u\n", base+3, base+4, base);
                fprintf(file, "f %u %u %u\n", base+1, base+5, base+6);  // Right
                fprintf(file, "f %u %u %u\n", base+1, base+6, base+2);

                overhang_count++;
            }
        }
    }

    fclose(file);

    printf("Overhang export complete: %u overhangs found, %u vertices\n", overhang_count, vertex_count);
}
