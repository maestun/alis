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
    
    u8 swapped[length];
    for (int i = 0; i < length; i++)
    {
        swapped[i] = buffer[i] + 0x80;
    }

    fwrite(&riffHeader, sizeof(WAVRIFFHeader), 1, file);
    fwrite(&fmtHeader, sizeof(WAVFmtHeader), 1, file);
    fwrite(&dataHeader, sizeof(WAVDataHeader), 1, file);
    fwrite(swapped, sizeof(u8), length, file);
    fclose(file);
}

void export_audio(u32 addr, const char *name)
{
    s8 type = xread8(addr);
    if (type == 1 || type == 2)
    {
        u32 freq = xread8(addr + 1);
        u32 length = ((alis.platform.kind == EPlatformPC && (alis.platform.uid == EGameColorado || alis.platform.uid == EGameWindsurfWilly || alis.platform.uid == EGameMadShow || alis.platform.uid == EGameLeFeticheMaya)) ? xpcread32(addr + 2) : xread32(addr + 2)) - 0x10;
        
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
