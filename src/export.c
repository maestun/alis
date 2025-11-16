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
// The height values are stored DIRECTLY - no altitude table lookup needed!
// This allows the terrain to be rendered as a simple triangulated mesh
void export_terrain_obj(s32 scene_addr, s32 render_context, const char *filepath)
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
    s32 terrain_data = image.atlland;  // Terrain height map grid (4 bytes per cell)
    s32 altitude_table = image.atalti; // Altitude lookup table (s16 values)

    // Scale factors for world-space coordinates
    float world_scale_x = 1.0f;
    float world_scale_y = 0.5f;  // Altitude table values already in world units
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
    // Terrain supports overhangs/bridges when adjacent cells have height difference:
    // If current_height < next_height, create overhang geometry
    u32 vertex_count = 0;

    // Store current row heights for overhang detection
    s16 *current_heights = malloc(sizeof(s16) * grid_width);
    s16 *next_heights = malloc(sizeof(s16) * grid_width);

    for (u16 y = 0; y < grid_height; y++)
    {
        for (u16 x = 0; x < grid_width; x++)
        {
            // Get altitude base pointer for this X column
            s32 cell_ptr = terrain_data + (x * 4);
            u32 altitude_base = xread32(cell_ptr);

            // Read 4 bytes at Y position as 2 x s16 values
            // s16 contains: upper byte = terrain type, lower byte = altitude index
            s16 cell0 = xread16(altitude_base + (y * 2) + 0);
            s16 cell1 = xread16(altitude_base + (y * 2) + 2);

            // Extract altitude indices from lower bytes
            u8 alt_index0 = (u8)(cell0 & 0xff);
            u8 alt_index1 = (u8)(cell1 & 0xff);

            // Look up heights in altitude table
            s16 height0 = xread16(altitude_table + (alt_index0 * 2));
            s16 height1 = xread16(altitude_table + (alt_index1 * 2));

            // Clamp positive heights to 0 (terrain doesn't go above ground)
            if (height0 > 0) height0 = 0;
            if (height1 > 0) height1 = 0;

            // Create first vertex at (x, height0, y)
            float pos_x1 = (float)x * world_scale_x;
            float pos_y1 = (float)height0 * world_scale_y;
            float pos_z1 = (float)y * world_scale_z;
            fprintf(file, "v %.6f %.6f %.6f\n", pos_x1, pos_y1, pos_z1);
            vertex_count++;

            // Create second vertex at (x+0.5, height1, y)
            float pos_x2 = ((float)x + 0.5f) * world_scale_x;
            float pos_y2 = (float)height1 * world_scale_y;
            float pos_z2 = (float)y * world_scale_z;
            fprintf(file, "v %.6f %.6f %.6f\n", pos_x2, pos_y2, pos_z2);
            vertex_count++;

            // Store height for overhang detection
            next_heights[x] = height0;  // Store first cell height for next iteration
        }

        // Detect and export overhang geometry
        // If next Y cell is higher than current, create overhang top surface
        if (y > 0)
        {
            for (u16 x = 0; x < grid_width; x++)
            {
                // Check if overhang exists: current < next
                if (next_heights[x] < current_heights[x])
                {
                    // Create overhang top vertices
                    // These create a horizontal surface at height of current cell
                    s16 overhang_height = next_heights[x];

                    float ov_x1 = (float)x * world_scale_x;
                    float ov_y = (float)overhang_height * world_scale_y;
                    float ov_z = (float)(y - 1) * world_scale_z + 0.5f;  // Midpoint
                    fprintf(file, "v %.6f %.6f %.6f\n", ov_x1, ov_y, ov_z);
                    vertex_count++;

                    float ov_x2 = ((float)x + 0.5f) * world_scale_x;
                    fprintf(file, "v %.6f %.6f %.6f\n", ov_x2, ov_y, ov_z);
                    vertex_count++;
                }
            }
        }

        // Swap buffers for next iteration
        s16 *tmp = current_heights;
        current_heights = next_heights;
        next_heights = tmp;
    }

    free(current_heights);
    free(next_heights);

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
            fprintf(file, "f %u %u %u\n", v0_a, v1_a, v2_a);
            face_count++;

            fprintf(file, "f %u %u %u\n", v1_a, v3_a, v2_a);
            face_count++;
        }
    }

    fclose(file);

    printf("Export complete: %u vertices, %u faces\n", vertex_count, face_count);
}

// Debug export: dumps raw terrain and altitude data for analysis
// This helps understand the exact memory layout and indexing
void export_terrain_debug(s32 scene_addr, s32 render_context, const char *filepath)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing\n", filepath);
        return;
    }

    // Get terrain dimensions
    u16 grid_width = xread16(scene_addr + 0x12) + 1;
    u16 grid_height = xread16(scene_addr + 0x14) + 1;

    // Load terrain data pointers
    s32 altitude_table = image.atalti;
    s32 terrain_data = image.atlland;

    // Get grid dimensions from render context
    u16 grid_dim_x = xread16(render_context - 0x294);  // Max X grid size
    u16 grid_dim_y = xread16(render_context - 0x292);  // Max Y grid size

    fprintf(file, "=== TERRAIN DATA DEBUG EXPORT ===\n\n");
    fprintf(file, "Scene Configuration:\n");
    fprintf(file, "  Grid dimensions (from scene_addr):\n");
    fprintf(file, "    Width:  %u cells (scene_addr + 0x12)\n", grid_width);
    fprintf(file, "    Height: %u cells (scene_addr + 0x14)\n", grid_height);
    fprintf(file, "  Render context dimensions:\n");
    fprintf(file, "    X bounds: %u (render_context - 0x294)\n", grid_dim_x);
    fprintf(file, "    Y bounds: %u (render_context - 0x292)\n", grid_dim_y);
    fprintf(file, "  Memory pointers:\n");
    fprintf(file, "    altitude_table (image.atalti): 0x%x\n", (u32)altitude_table);
    fprintf(file, "    terrain_data (image.atlland):  0x%x\n\n", (u32)terrain_data);

    // Sample terrain_data at various grid positions
    fprintf(file, "=== TERRAIN DATA GRID SAMPLES ===\n");
    fprintf(file, "Sampling terrain_data at selected X positions (each entry is 4 bytes)\n");
    fprintf(file, "Format: terrain_data[x*4] = 32-bit pointer value\n\n");

    for (u16 x = 0; x < (grid_width < 32 ? grid_width : 32); x++)
    {
        s32 cell_ptr = terrain_data + (x * 4);
        u32 altitude_ptr = xread32(cell_ptr);
        fprintf(file, "X=%3u: offset=0x%05x value=0x%08x (altitude_ptr)\n",
                x, (x * 4), altitude_ptr);
    }
    fprintf(file, "\n");

    // Detailed terrain cell dump for first few columns
    fprintf(file, "=== DETAILED TERRAIN CELL DATA ===\n");
    fprintf(file, "For each X column, reading altitude values at different Y positions\n\n");

    for (u16 x = 0; x < (grid_width < 8 ? grid_width : 8); x++)
    {
        fprintf(file, "Column X=%u:\n", x);
        s32 cell_ptr = terrain_data + (x * 4);
        u32 altitude_base = xread32(cell_ptr);

        fprintf(file, "  Base altitude pointer: 0x%08x\n", altitude_base);

        // Sample Y positions in this column
        for (u16 y = 0; y < (grid_dim_y < 16 ? grid_dim_y : 16); y++)
        {
            // Try the calculation from doland: iVar10 + (y * 4)
            s32 addr = altitude_base + (y * 4);
            u32 val = xread32(addr);

            fprintf(file, "    Y=%2u: addr=0x%08x -> u32=0x%08x", y, (u32)addr, val);

            // Also read as s16 at offset 0 and 1
            s16 as_s16_0 = xread16(addr + 0);
            u8 as_u8_0 = xread8(addr + 0);
            u8 as_u8_1 = xread8(addr + 1);
            fprintf(file, " [u8_0=0x%02x u8_1=0x%02x s16_0=0x%04x]\n",
                    as_u8_0, as_u8_1, (u16)as_s16_0);
        }
        fprintf(file, "\n");
    }

    // Dump altitude table structure
    fprintf(file, "=== ALTITUDE TABLE SAMPLES ===\n");
    fprintf(file, "Altitude table is organized as s16 values (2 bytes each)\n");
    fprintf(file, "Storage: segments of 0x200 bytes (256 entries of 2 bytes = 0x100 values)\n\n");

    // Sample the altitude table directly
    fprintf(file, "Direct altitude table lookup samples (first 0x100 entries):\n");
    for (u16 i = 0; i < (0x100 < 64 ? 0x100 : 64); i++)
    {
        s16 alt_val = xread16(altitude_table + (i * 2));
        if (i % 8 == 0)
            fprintf(file, "\n");
        fprintf(file, "[%3u]=0x%04x ", i, (u16)alt_val);
    }
    fprintf(file, "\n\n");

    // Sample different altitude table segments (0x200 byte stride)
    fprintf(file, "=== ALTITUDE TABLE SEGMENT STRUCTURE ===\n");
    fprintf(file, "Segments are at 0x200 byte intervals (0x100 s16 values per segment)\n\n");

    for (u16 seg = 0; seg < 4; seg++)
    {
        fprintf(file, "Segment %u (offset +0x%x):\n", seg, seg * 0x200);
        s32 seg_addr = altitude_table + (seg * 0x200);

        fprintf(file, "  First 16 values:  ");
        for (u16 i = 0; i < 16; i++)
        {
            s16 val = xread16(seg_addr + (i * 2));
            fprintf(file, "[%u]=0x%04x ", i, (u16)val);
        }
        fprintf(file, "\n");

        fprintf(file, "  Last 16 values:   ");
        for (u16 i = 0xf0; i < 0x100; i++)
        {
            s16 val = xread16(seg_addr + (i * 2));
            fprintf(file, "[%u]=0x%04x ", i, (u16)val);
        }
        fprintf(file, "\n\n");
    }

    // Cross-reference: sample a terrain cell and trace through altitude lookup
    fprintf(file, "=== TRACED ALTITUDE LOOKUP EXAMPLE ===\n");
    fprintf(file, "Following the lookup path for grid position (10, 5):\n\n");

    u16 test_x = 10;
    u16 test_y = 5;

    fprintf(file, "1. Get terrain cell pointer:\n");
    fprintf(file, "   terrain_data + (x * 4) = 0x%x + (%u * 4) = 0x%x\n",
            (u32)terrain_data, test_x, (u32)(terrain_data + (test_x * 4)));

    s32 test_cell_ptr = terrain_data + (test_x * 4);
    u32 test_altitude_base = xread32(test_cell_ptr);
    fprintf(file, "   altitude_base = xread32(0x%x) = 0x%08x\n",
            (u32)test_cell_ptr, test_altitude_base);

    fprintf(file, "\n2. Access Y coordinate in altitude column:\n");
    fprintf(file, "   Option A: altitude_base + (y * 4) = 0x%x + (%u * 4) = 0x%x\n",
            test_altitude_base, test_y, test_altitude_base + (test_y * 4));

    s32 test_addr_a = test_altitude_base + (test_y * 4);
    u32 test_val_a = xread32(test_addr_a);
    fprintf(file, "   Result: 0x%08x\n", test_val_a);

    fprintf(file, "   Option B: altitude_base + (y * 2) = 0x%x + (%u * 2) = 0x%x\n",
            test_altitude_base, test_y, test_altitude_base + (test_y * 2));

    s32 test_addr_b = test_altitude_base + (test_y * 2);
    s16 test_val_b = xread16(test_addr_b);
    fprintf(file, "   Result: 0x%04x (as s16)\n", (u16)test_val_b);

    fprintf(file, "\n3. Read individual bytes:\n");
    u8 byte0 = xread8(test_addr_b);
    u8 byte1 = xread8(test_addr_b + 1);
    fprintf(file, "   Byte 0: 0x%02x\n", byte0);
    fprintf(file, "   Byte 1: 0x%02x\n", byte1);

    fprintf(file, "\n4. Also check 4-byte alignment path:\n");
    s32 test_addr_4 = test_altitude_base + (test_y * 4);
    u32 test_val_4 = xread32(test_addr_4);
    u16 test_val_4_lo = (u16)test_val_4;
    u16 test_val_4_hi = (u16)(test_val_4 >> 16);
    fprintf(file, "   Address: 0x%x\n", (u32)test_addr_4);
    fprintf(file, "   Value:   0x%08x\n", test_val_4);
    fprintf(file, "   Low word:  0x%04x\n", test_val_4_lo);
    fprintf(file, "   High word: 0x%04x\n", test_val_4_hi);

    fclose(file);

    printf("Debug export complete: %s\n", filepath);
}

// Enhanced debug export focusing on endianness issues
void export_terrain_debug_endian(s32 scene_addr, s32 render_context, const char *filepath)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing\n", filepath);
        return;
    }

    u16 grid_width = xread16(scene_addr + 0x12) + 1;
    u16 grid_height = xread16(scene_addr + 0x14) + 1;

    s32 terrain_data = image.atlland;
    s32 altitude_table = image.atalti;

    fprintf(file, "=== ENDIANNESS DEBUG EXPORT ===\n\n");
    fprintf(file, "Grid: %u x %u cells\n\n", grid_width, grid_height);

    // Show all altitude table values with their indices
    fprintf(file, "=== COMPLETE ALTITUDE TABLE ===\n");
    fprintf(file, "Using xread16() with endianness conversion:\n\n");
    for (u16 i = 0; i < 0x100; i++)
    {
        s16 val = xread16(altitude_table + (i * 2));
        if (i % 8 == 0)
            fprintf(file, "\n");
        fprintf(file, "[%3u]=0x%04x(%6d) ", i, (u16)val, val);
    }
    fprintf(file, "\n\n");

    // Dump complete column X=0
    fprintf(file, "=== COMPLETE COLUMN X=0 (all 127 Y values) ===\n");
    fprintf(file, "Format: Y idx, xread32() result, manual byte extraction, two xread16() reads\n\n");

    s32 cell_ptr = terrain_data + (0 * 4);
    u32 altitude_base = xread32(cell_ptr);

    fprintf(file, "Column base pointer: 0x%08x\n\n", altitude_base);

    for (u16 y = 0; y < grid_height && y < 127; y++)
    {
        // Method 1: Use xread32 directly
        u32 cell_val_xread32 = xread32(altitude_base + (y * 4));

        // Method 2: Extract bytes manually with xread8
        u8 byte0 = xread8(altitude_base + (y * 4) + 0);
        u8 byte1 = xread8(altitude_base + (y * 4) + 1);
        u8 byte2 = xread8(altitude_base + (y * 4) + 2);
        u8 byte3 = xread8(altitude_base + (y * 4) + 3);

        // Method 3: Read as two s16 values
        s16 val_s16_0 = xread16(altitude_base + (y * 4) + 0);
        s16 val_s16_2 = xread16(altitude_base + (y * 4) + 2);

        fprintf(file, "Y=%3u: xread32=0x%08x  bytes=[%02x %02x %02x %02x]  s16s=[0x%04x 0x%04x]\n",
                y, cell_val_xread32, byte0, byte1, byte2, byte3,
                (u16)val_s16_0, (u16)val_s16_2);

        // Show what altitude lookups would give for the byte values
        if (y < 16)  // Only first 16 rows
        {
            s16 alt0 = xread16(altitude_table + (byte0 * 2));
            s16 alt1 = xread16(altitude_table + (byte2 * 2));
            fprintf(file, "      Altitude lookups: byte0(%u)->0x%04x  byte2(%u)->0x%04x\n",
                    byte0, (u16)alt0, byte2, (u16)alt1);
        }
    }

    fclose(file);
    printf("Endian debug export complete: %s\n", filepath);
}

// Detailed column analysis: trace exact data flow from terrain_data through altitude lookups
void export_terrain_column_analysis(s32 scene_addr, s32 render_context, const char *filepath)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        printf("Error: Could not open file %s for writing\n", filepath);
        return;
    }

    u16 grid_width = xread16(scene_addr + 0x12) + 1;
    u16 grid_height = xread16(scene_addr + 0x14) + 1;

    s32 terrain_data = image.atlland;
    s32 altitude_table = image.atalti;

    fprintf(file, "=== TERRAIN COLUMN DATA ANALYSIS ===\n\n");
    fprintf(file, "Grid dimensions: %u x %u\n", grid_width, grid_height);
    fprintf(file, "terrain_data base: 0x%08x\n", (u32)terrain_data);
    fprintf(file, "altitude_table base: 0x%08x\n\n", (u32)altitude_table);

    // Analyze multiple columns to identify patterns
    for (u16 col_x = 0; col_x < (grid_width < 4 ? grid_width : 4); col_x++)
    {
        fprintf(file, "=== COLUMN X=%u ===\n", col_x);
        fprintf(file, "terrain_data[%u*4] address: 0x%08x\n", col_x, (u32)(terrain_data + (col_x * 4)));

        // Get the base pointer for this column
        u32 altitude_base = xread32(terrain_data + (col_x * 4));
        fprintf(file, "altitude_base pointer (from terrain_data): 0x%08x\n\n", altitude_base);

        fprintf(file, "Y   addr         Raw Hex Bytes [0][1][2][3]     xread16 pairs      Byte variations\n");
        fprintf(file, "                                              [+0] [+2]         byte0 byte1 byte2 byte3\n");
        fprintf(file, "--- --------- -------- -------- -------- -------- -------- -------- --- --- --- ---\n");

        // Dump all Y values with hex analysis
        for (u16 y = 0; y < grid_height; y++)
        {
            u32 cell_addr = altitude_base + (y * 4);

            // Read as individual bytes
            u8 byte0 = xread8(cell_addr + 0);
            u8 byte1 = xread8(cell_addr + 1);
            u8 byte2 = xread8(cell_addr + 2);
            u8 byte3 = xread8(cell_addr + 3);

            // Read as s16 pairs
            s16 val_s16_0 = xread16(cell_addr + 0);
            s16 val_s16_2 = xread16(cell_addr + 2);

            fprintf(file, "%3u 0x%08x %02x %02x %02x %02x  0x%04x 0x%04x  ",
                    y, (u32)cell_addr, byte0, byte1, byte2, byte3,
                    (u16)val_s16_0, (u16)val_s16_2);

            // Show which bytes vary
            fprintf(file, "%02x %02x %02x %02x", byte0, byte1, byte2, byte3);

            // Try altitude lookups for each byte position
            fprintf(file, "  [");
            s16 alt0 = xread16(altitude_table + (byte0 * 2));
            fprintf(file, "%d", alt0);
            fprintf(file, "]");

            fprintf(file, "\n");

            // Every 8 rows, show summary
            if ((y + 1) % 16 == 0)
            {
                fprintf(file, "--- --------- -------- -------- -------- -------- -------- -------- --- --- --- ---\n");
            }
        }

        fprintf(file, "\n");
    }

    // Now analyze which byte is actually the varying height index
    fprintf(file, "\n=== BYTE VARIATION ANALYSIS ===\n");
    fprintf(file, "For each column, identify which byte(s) vary as Y changes:\n\n");

    for (u16 col_x = 0; col_x < (grid_width < 4 ? grid_width : 4); col_x++)
    {
        fprintf(file, "Column X=%u:\n", col_x);

        u32 altitude_base = xread32(terrain_data + (col_x * 4));

        // Track min/max for each byte position
        u8 byte0_min = 0xff, byte0_max = 0x00;
        u8 byte1_min = 0xff, byte1_max = 0x00;
        u8 byte2_min = 0xff, byte2_max = 0x00;
        u8 byte3_min = 0xff, byte3_max = 0x00;

        // Collect statistics
        for (u16 y = 0; y < grid_height; y++)
        {
            u32 cell_addr = altitude_base + (y * 4);
            u8 byte0 = xread8(cell_addr + 0);
            u8 byte1 = xread8(cell_addr + 1);
            u8 byte2 = xread8(cell_addr + 2);
            u8 byte3 = xread8(cell_addr + 3);

            if (byte0 < byte0_min) byte0_min = byte0;
            if (byte0 > byte0_max) byte0_max = byte0;
            if (byte1 < byte1_min) byte1_min = byte1;
            if (byte1 > byte1_max) byte1_max = byte1;
            if (byte2 < byte2_min) byte2_min = byte2;
            if (byte2 > byte2_max) byte2_max = byte2;
            if (byte3 < byte3_min) byte3_min = byte3;
            if (byte3 > byte3_max) byte3_max = byte3;
        }

        fprintf(file, "  Byte 0: range 0x%02x-0x%02x (varies: %s)\n",
                byte0_min, byte0_max, (byte0_min != byte0_max) ? "YES" : "no");
        fprintf(file, "  Byte 1: range 0x%02x-0x%02x (varies: %s)\n",
                byte1_min, byte1_max, (byte1_min != byte1_max) ? "YES" : "no");
        fprintf(file, "  Byte 2: range 0x%02x-0x%02x (varies: %s)\n",
                byte2_min, byte2_max, (byte2_min != byte2_max) ? "YES" : "no");
        fprintf(file, "  Byte 3: range 0x%02x-0x%02x (varies: %s)\n",
                byte3_min, byte3_max, (byte3_min != byte3_max) ? "YES" : "no");

        fprintf(file, "\n");
    }

    // Test altitude lookups for promising indices
    fprintf(file, "\n=== ALTITUDE TABLE LOOKUP TEST ===\n");
    fprintf(file, "Testing which byte index produces sensible altitude values:\n\n");

    u32 col0_base = xread32(terrain_data + (0 * 4));

    fprintf(file, "Sample from Column X=0, Y=0-15:\n");
    fprintf(file, "Y   byte0  alt[b0]   byte1  alt[b1]   byte2  alt[b2]   byte3  alt[b3]\n");
    fprintf(file, "--- ------ ------- ------ ------- ------ ------- ------ -------\n");

    for (u16 y = 0; y < 16 && y < grid_height; y++)
    {
        u32 cell_addr = col0_base + (y * 4);
        u8 byte0 = xread8(cell_addr + 0);
        u8 byte1 = xread8(cell_addr + 1);
        u8 byte2 = xread8(cell_addr + 2);
        u8 byte3 = xread8(cell_addr + 3);

        s16 alt0 = xread16(altitude_table + (byte0 * 2));
        s16 alt1 = xread16(altitude_table + (byte1 * 2));
        s16 alt2 = xread16(altitude_table + (byte2 * 2));
        s16 alt3 = xread16(altitude_table + (byte3 * 2));

        fprintf(file, "%3u 0x%02x    %6d  0x%02x    %6d  0x%02x    %6d  0x%02x    %6d\n",
                y, byte0, alt0, byte1, alt1, byte2, alt2, byte3, alt3);
    }

    // Show color/property data from barland references
    fprintf(file, "\n=== COLOR/PROPERTY DATA (from barland) ===\n");
    fprintf(file, "barland reads xread8(terrain_cell) and xread8(terrain_cell+1)\n");
    fprintf(file, "These appear to be color/terrain type indices.\n");
    fprintf(file, "Showing byte0 and byte1 from sample cells:\n\n");

    u32 sample_addr = col0_base;
    fprintf(file, "Cell at offset +0: byte0=0x%02x (color?) byte1=0x%02x (property?)\n",
            xread8(sample_addr + 0), xread8(sample_addr + 1));
    fprintf(file, "Cell at offset +4: byte0=0x%02x (color?) byte1=0x%02x (property?)\n",
            xread8(sample_addr + 4), xread8(sample_addr + 5));
    fprintf(file, "Cell at offset +8: byte0=0x%02x (color?) byte1=0x%02x (property?)\n",
            xread8(sample_addr + 8), xread8(sample_addr + 9));

    fclose(file);
    printf("Column analysis complete: %s\n", filepath);
}
