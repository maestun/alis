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

#include "image.h"

#include <stdbool.h>

#include "alis.h"
#include "alis_private.h"
#include "debug.h"
#include "mem.h"
#include "screen.h"
#include "utils.h"


#define FRAME_TICKS (1000000 / 50) // 25 fps looks about right in logo animation

#define DEBUG_CHECK 0

#if DEBUG_CHECK > 0
# define VERIFYINTEGRITY verifyintegrity()
#else
# define VERIFYINTEGRITY
#endif


u8 cga_palette[] = {
    0x00, 0x00, 0x00,
    0x55, 0xff, 0xff,
    0xff, 0x55, 0xff,
    0xff, 0xff, 0xff };

u8 ega_palette[] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0xaa,
    0x00, 0xaa, 0x00,
    0x00, 0xaa, 0xaa,
    0xaa, 0x00, 0x00,
    0xaa, 0x00, 0xaa,
    0xaa, 0x55, 0x00,
    0xaa, 0xaa, 0xaa,
    0x55, 0x55, 0x55,
    0x55, 0x55, 0xff,
    0x55, 0xff, 0x55,
    0x55, 0xff, 0xff,
    0xff, 0x55, 0x55,
    0xff, 0x55, 0xff,
    0xff, 0xff, 0x55,
    0xff, 0xff, 0xff};

u8 masks[4] = { 0b11000000, 0b00110000, 0b00001100, 0b00000011 };
u8 rots[4] = { 6, 4, 2, 0 };

sImage image = {
    .backprof = 0,
    .numelem = 0,
    .invert_x = 0,
    .depx = 0,
    .depy = 0,
    .depz = 0,
    .oldcx = 0,
    .oldcy = 0,
    .oldcz = 0,
    .spag = 0,
    .wpag = 0,
    .sback = 0,
    .wback = 0,
    .cback = 0,
    .pback = 0
};


void draw_mac_rect(sRect *pos, sRect *bmp, u8 color);
void draw_cga_rect(sRect *pos, sRect *bmp, u8 color);
void draw_rect(sRect *pos, sRect *bmp, u8 color);
void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip);
void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip);
void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_fli_video(u8 *bitmap);
void draw_map(sSprite *sprite, u32 mapaddr, sRect lim);


#pragma mark -
#pragma mark Palette management


void topalet(void)
{
    image.ftopal = 0;

    if (image.palc == 1)
    {
        memcpy(image.ampalet, image.atpalet, 768);
    }
    else
    {
        for (s32 i = 0; i < 768; i++)
        {
            if (image.ampalet[i] != image.atpalet[i])
                image.ampalet[i] = image.atpalet[i] + image.dpalet[i] * image.palc;
        }
    }
    
    image.ftopal = 1;

    set_update_cursor();
}

void topalette(u8 *paldata, s32 duration)
{
    if (alis.platform.kind == EPlatformMac)
    {
        memset(image.atpalet, 0, 3);
        memset(image.ampalet, 0, 3);
        memset(image.atpalet + 3, 0xff, 765);
        memset(image.ampalet + 3, 0xff, 765);
        image.ftopal = 0xff;
        return;
    }
    else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
    {
        memcpy(image.atpalet, cga_palette, sizeof(cga_palette));
        memcpy(image.ampalet, cga_palette, sizeof(cga_palette));
        return;
    }
    
    selpalet();

    s16 colors = paldata[1];
    if (colors == 0) // 4 bit palette
    {
        image.palc = 0;
        u8 *palptr = &paldata[2];

        s16 to = 0;
        if (alis.platform.kind == EPlatformAmiga)
        {
            for (s32 i = 0; i < 16; i++)
            {
                image.atpalet[to++] = (palptr[i * 2 + 0] & 0b00001111) << 4;
                image.atpalet[to++] = (palptr[i * 2 + 1] >> 4) << 4;
                image.atpalet[to++] = (palptr[i * 2 + 1] & 0b00001111) << 4;
            }
        }
        else
        {
            for (s32 i = 0; i < 16; i++)
            {
                image.atpalet[to++] = (palptr[i * 2 + 0] & 0b00000111) << 5;
                image.atpalet[to++] = (palptr[i * 2 + 1] >> 4) << 5;
                image.atpalet[to++] = (palptr[i * 2 + 1] & 0b00000111) << 5;
            }
        }
    }
    else // 8 bit palette
    {
        if (alis.platform.kind == EPlatformAmiga)
        {
            image.palc = 0;
            u8 *palptr = &paldata[2];

            s16 to = 0;

            for (s32 i = 0; i < 32; i++)
            {
                image.atpalet[to++] = (palptr[i * 2 + 0] & 0b00001111) << 4;
                image.atpalet[to++] = (palptr[i * 2 + 1] >> 4) << 4;
                image.atpalet[to++] = (palptr[i * 2 + 1] & 0b00001111) << 4;
            }
        }
        else
        {
            image.palc = 0;
            u8 offset = paldata[2];
            memcpy(image.atpalet + (offset * 3), paldata + 4, (1 + colors) * 3);
        }
    }
    
    if (image.fdarkpal && image.thepalet == 0)
    {
        u16 *dkpalptr = (u16 *)image.dkpalet;
        s16 colors = alis.platform.bpp <= 4 ? 16 : 256;

        s16 to = 0;
        
        for (s32 i = 0; i < colors; i++, dkpalptr += 3)
        {
            image.atpalet[to++] *= (dkpalptr[0] / 256.0);
            image.atpalet[to++] *= (dkpalptr[1] / 256.0);
            image.atpalet[to++] *= (dkpalptr[2] / 256.0);
        }
    }

    image.thepalet = 0;
    image.defpalet = 0;
    
    if (duration != 0)
    {
        selpalet();

        for (s32 i = 0; i < 768; i++)
        {
            image.dpalet[i] = (image.ampalet[i] - image.atpalet[i]) / (float)duration;
        }

        image.palt = 1;
        image.palt0 = 1;
        image.palc = duration;
    }
    else
    {
        memcpy(image.ampalet, image.atpalet, 768);
        image.ftopal = 0xff;
    }
    
    set_update_cursor();
}

void toblackpal(s16 duration)
{
    selpalet();

    memset(image.atpalet, 0, 768);
    
    image.thepalet = 0;
    image.defpalet = 0;

    if (duration == 0)
    {
        memset(image.ampalet, 0, 768);
        image.ftopal = 0xff;
        image.palc = 0;
    }
    else
    {
        selpalet();

        for (s32 i = 0; i < 768; i++)
        {
            image.dpalet[i] = (image.ampalet[i] - image.atpalet[i]) / (float)duration;
        }

        image.palt = 1;
        image.palt0 = 1;
        image.palc = duration;
    }
}

void savepal(s16 mode)
{
    if (mode < 0 && -3 < mode)
    {
        u8 *tgt = (mode != -1) ? image.svpalet2 : image.svpalet;
        memcpy(tgt, image.tpalet, 768);
    }
}

void restorepal(s16 mode, s32 duration)
{
    // not used in some games (Metal Mutant, ...)
    if (alis.platform.uid == EGameMetalMutant)
    {
        return;
    }

    u8 *src = (mode != -1) ? image.svpalet2 : image.svpalet;
    memcpy(image.tpalet, src, 768);

    image.thepalet = 0;
    image.defpalet = 0;
    
    if (duration == 0)
    {
        memcpy(image.mpalet, src, 768);
        image.ftopal = 0xff;
        image.palc = 0;
    }
    else
    {
        for (s32 i = 0; i < 768; i++)
        {
            image.dpalet[i] = (image.mpalet[i] - image.tpalet[i]) / (float)duration;
        }

        topalet();
        image.palt = 1;
        image.palt0 = 1;
        image.palc = duration;
    }
}

void selpalet(void)
{
    if (alis.platform.bpp != 8)
    {
        s16 offset = image.thepalet * 64 * 3;
        image.ampalet = image.mpalet + offset;
        image.atpalet = image.tpalet + offset;
    }
}

void linepal(void)
{
    s32 begline = 0xc6;
    s16 endline = 0x2f;

    s16 line;
    s16 dummy;
    
    s16 *tlinepal_ptr = image.tlinepal;
    s16 *palentry = image.firstpal;
    
    for (int i = 0; i < 4; i++)
    {
        line = tlinepal_ptr[0];
        if (alis.platform.height < line)
            break;
        
        dummy = line == 0 ? line : ((u16)(line * begline) >> 8) + endline;
        
        palentry[0] = dummy;
        palentry[1] = line;
        *(u8 **)&(palentry[2]) = image.mpalet + (tlinepal_ptr[1] * 64 * 3);

        tlinepal_ptr += 2;
        palentry += 2 + (sizeof(u8 *) >> 1);
    }
    
    *(s16 *)(palentry + 0) = 0xff;
    *(s16 *)(palentry + 2) = 0;
    *(u8 **)(palentry + 4) = image.mpalet;
}

void setlinepalet(void) {
    
    if (alis.varD7 < 0)
    {
        image.flinepal = 0;
        image.tlinepal[0] = 0;
        image.tlinepal[1] = 0;
        image.tlinepal[2] = 0xff;
        image.tlinepal[3] = 0;
        linepal();
        return;
    }
    
    if (image.logy2 <= alis.varD7)
    {
        alis.varD7 = image.logy2;
    }

    s16 *tlinepal_ptr = image.tlinepal;
    s16 *prevtlpal_ptr;
    
    do
    {
        prevtlpal_ptr = tlinepal_ptr;
        if (alis.varD7 == prevtlpal_ptr[0])
        {
            if (alis.varD6 == prevtlpal_ptr[1])
                return;
            
            prevtlpal_ptr[1] = alis.varD6;
            image.flinepal = 1;
            linepal();
            return;
        }
        
        tlinepal_ptr = prevtlpal_ptr + 2;
    }
    while (prevtlpal_ptr[0] < alis.platform.height);
    
    // check whether we are not using too many palettes
    if (prevtlpal_ptr - (image.tlinepal + 8) < 0)
    {
        do
        {
            prevtlpal_ptr = tlinepal_ptr;
            prevtlpal_ptr[0] = prevtlpal_ptr[-2];
            prevtlpal_ptr[1] = prevtlpal_ptr[-1];
            tlinepal_ptr = prevtlpal_ptr - 2;
        }
        while ((s16)(alis.varD7 - prevtlpal_ptr[0]) < 0);
        
        prevtlpal_ptr[0] = alis.varD7;
        prevtlpal_ptr[1] = alis.varD6;
        image.flinepal = 1;
        linepal();
    }
}

void setmpalet(void)
{
    image.ftopal = 0xff;
    image.thepalet = 0;
    image.defpalet = 0;
}


#pragma mark -
#pragma mark Sprite management


void printelem(void)
{
    s32 cursprit = image.debsprit;
    cursprit += 0x78;
    cursprit += 0x60;

    do
    {
        cursprit += 0x30;
    }
    while (cursprit < image.debsprit + 16 * 0x30);
}

void log_sprites(void)
{
    u8 result = true;

    if (image.libsprit == 0)
    {
        debug(EDebugError, "image.libsprit = 0!\n");
        result = false;
    }
    
    debug(EDebugInfo, "  list\n");

    u16 curidx;
    u16 scsprite = screen.ptscreen;
    
    while (scsprite != 0)
    {
        // if ((get_scr_state(scsprite) & 0x40) == 0)
        {
            debug(EDebugInfo, "  %s screen [0x%.4x]\n", (get_scr_state(scsprite) & 0x40) == 0 ? "visible" : "hidden", ELEMIDX(scsprite));

            u8 *bitmap = 0;
            sSprite *sprite;
            u16 lastidx = 0;
            for (curidx = get_scr_screen_id(scsprite); curidx != 0; curidx = SPRITE_VAR(curidx)->link)
            {
                lastidx = curidx;
                sprite = SPRITE_VAR(curidx);

                sAlisScriptLive *script = ENTSCR(sprite->script_ent);
                
                s32 addr = 0;
                s32 index = -1;
                
                bool deleted = false;
                
                if (script->vram_org)
                {
                    u8 *ptr = alis.mem + get_0x14_script_org_offset(script->vram_org);
                    s32 l = read32(ptr + 0xe);
                    s32 e = read16(ptr + l + 4);
                    
                    sAlisScriptLive *prev = alis.script;
                    alis.script = script;
                    
                    for (s32 i = 0; i < e; i++)
                    {
                        addr = adresdes(i);
                        if (sprite->data == addr)
                        {
                            index = i;
                            break;
                        }
                    }
                    
                    alis.script = prev;
                }
                else
                {
                    deleted = true;
                }
                
                u8 type = 0;
                s16 width = -1;
                s16 height = -1;
                u32 spnewad = sprite->newad;
                if (spnewad)
                {
                    bitmap = (alis.mem + spnewad + xread32(spnewad));
                    if (bitmap)
                    {
                        if (bitmap[0] == 1)
                            type = 1;

                        width = read16(bitmap + 2);
                        height = read16(bitmap + 4);
                    }
                }

                debug(EDebugInfo, "  %s %.2x %.2x %.4x type: %c idx: %.3d [x:%d y:%d d:%d w:%d h:%d] %s\n", deleted ? "!!" : "  ", (u8)sprite->state, (u8)sprite->numelem, ELEMIDX(curidx), type ? 'R' : 'B', index, sprite->newx, sprite->newy, sprite->newd, width, height, script->name);
            }
        }

        scsprite = get_scr_to_next(scsprite);
    }

    if (!result)
    {
        debug(EDebugError, "INTEGRITY COMPROMISED!\n");
    }
}

u8 verifyintegrity(void)
{
    u8 result = true;

    if (image.libsprit == 0)
    {
        debug(EDebugError, "ERROR: image.libsprit = 0!\n");
        result = false;
    }
    
    sSprite *cursprvar = NULL;
    
    debug(EDebugVerbose, "  list\n");

    u16 previdx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx != 0)
    {
        cursprvar = SPRITE_VAR(curidx);
        
        s16 link = cursprvar->link;
        if (link == curidx)
        {
            debug(EDebugError, "ERROR: link = 0\n");
            result = false;
        }
        
        if (link)
        {
            s16 link2 = SPRITE_VAR(link)->link;
            if (link2 == curidx)
            {
                debug(EDebugError, "ERROR: infinite loop\n");
                result = false;
                cursprvar->link = 0;
            }
        }

        previdx = curidx;
        curidx = cursprvar->to_next;
    }

    if (!result)
    {
        debug(EDebugError, "INTEGRITY COMPROMISED!\n");
    }
 
    return result;
}

void inisprit(void)
{
    image.debsprit = 0;
    image.finsprit = 0xffff;

    s32 cursprit = image.debsprit;
    image.tvsprite = 0x8000;
    image.basesprite = image.debsprit + 0x8000;
    *(s32 *)(image.spritemem + cursprit + 0x0c) = image.logx1;
    *(s16 *)(image.spritemem + cursprit + 0x16) = image.logx2;
    *(s16 *)(image.spritemem + cursprit + 0x18) = image.logy2;
    *(s8 *) (image.spritemem + cursprit + 0x29) = image.logy1;
    image.backsprite = 0x000e; // 0x8028;
    image.texsprite = 0x0042; // 0x8050;
    *(s32 *)(image.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (image.spritemem + cursprit + 0x50) = 0xfe;
    alis.mousflag = 0;
    image.libsprit = 0x78; // 0x8078;
    cursprit += 0x78;
    
    sSprite *sprite = SPRITE_VAR(cursprit);
    for (; cursprit < image.finsprit; sprite = SPRITE_VAR(cursprit), cursprit += 0x30)
    {
        sprite->to_next = cursprit + 0x30;
    }

    sprite->to_next = 0;
}

u8 searchelem(u16 *curidx, u16 *previdx)
{
    *curidx = get_0x18_unknown(alis.script->vram_org);
    if (*curidx != 0)
    {
        sSprite *cursprvar = NULL;
        s16 screen_id;
        s8 num;

        do
        {
            cursprvar = SPRITE_VAR(*curidx);
            screen_id = cursprvar->screen_id;
            if (screen_id <= get_0x16_screen_id(alis.script->vram_org))
            {
                if (get_0x16_screen_id(alis.script->vram_org) != screen_id)
                    break;
                
                num = cursprvar->numelem;
                if (num <= image.numelem)
                {
                    if (image.numelem == num)
                    {
                        return 1;
                    }
                    
                    break;
                }
            }
            
            *previdx = *curidx;
            *curidx = cursprvar->to_next;
        }
        while (*curidx != 0);
    }
    
    return 0;
}

s8 searchtete(u16 *curidx, u16 *previdx)
{
    *previdx = 0;
    *curidx = get_0x18_unknown(alis.script->vram_org);
    if (*curidx != 0)
    {
        s16 screenid = get_0x16_screen_id(alis.script->vram_org);
        
        do
        {
            sSprite *sprite = SPRITE_VAR(*curidx);
            s16 sprscid = sprite->screen_id;
            if (sprscid <= screenid)
            {
                if (screenid == sprscid)
                    return screenid == sprscid;
                
                break;
            }
            
            *previdx = *curidx;
            *curidx = sprite->to_next;
        }
        while (*curidx != 0);
    }
    
    return 0;
}

u8 testnum(u16 *curidx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

u8 nextnum(u16 *curidx, u16 *previdx)
{
    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

void createlem(u16 *curidx, u16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(image.libsprit);

    if (image.libsprit != 0)
    {
        u16 sprit = image.libsprit;
        u16 nextsprit = cursprvar->to_next;

        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, image.libsprit);
            image.libsprit = nextsprit;
        }
        else
        {
            sSprite *prevsprvar = SPRITE_VAR(*previdx);
            prevsprvar->to_next = image.libsprit;
            image.libsprit = nextsprit;
        }

        s16 scrnidx = get_scr_screen_id((get_0x16_screen_id(alis.script->vram_org)));
        sSprite *scrnsprvar = SPRITE_VAR(scrnidx);

        cursprvar->state = -1;
        cursprvar->numelem = image.numelem;
        cursprvar->screen_id = get_0x16_screen_id(alis.script->vram_org);
        cursprvar->to_next = *curidx;
        cursprvar->link = scrnsprvar->link;
        scrnsprvar->link = sprit;

        *curidx = sprit;
    }

//    VERIFYINTEGRITY;
}

void delprec(u16 elemidx)
{
    sSprite *cursprvar = SPRITE_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;
    if (elemidx == get_scr_screen_id(scridx))
    {
        set_scr_screen_id(scridx, cursprvar->screen_id);
        return;
    }

    u16 spridx = get_scr_screen_id(scridx);
    sSprite *tmpsprvar = SPRITE_VAR(spridx);
    while (tmpsprvar != NULL && elemidx != tmpsprvar->link)
    {
        spridx = tmpsprvar->link;
        tmpsprvar = SPRITE_VAR(spridx);
    }
    
    if (tmpsprvar != NULL)
    {
        tmpsprvar->link = cursprvar->link;
    }
}

void killelem(u16 *curidx, u16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
//    sAlisScriptLive *s = ENTSCR(cursprvar->script_ent);
//    u8 *resourcedata = alis.mem + cursprvar->data;
//    s16 width = read16(resourcedata + 2);
//    s16 height = read16(resourcedata + 4);
//    printf(" killelem: %d x %d [%s] ", width, height, s->name);
    
    if (alis.ferase == 0 && -1 < cursprvar->state)
    {
        cursprvar->state = 1;
        
        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, cursprvar->to_next);
        }
        else
        {
            SPRITE_VAR(*previdx)->to_next = cursprvar->to_next;
        }
    }
    else
    {
        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, cursprvar->to_next);
        }
        else
        {
            SPRITE_VAR(*previdx)->to_next = cursprvar->to_next;
        }
        
        cursprvar->to_next = image.libsprit;
        image.libsprit = *curidx;
        
        delprec(*curidx);
    }

    if (*previdx == 0)
    {
        *curidx = get_0x18_unknown(alis.script->vram_org);
//            VERIFYINTEGRITY;
        return;
    }
    
    *curidx = SPRITE_VAR(*previdx)->to_next;
//    VERIFYINTEGRITY;
}

void getelem(u16 *newidx, u16 *oldidx)
{
    u8 ret = searchelem(newidx, oldidx);
    if (ret)
    {
        do
        {
            if (SPRITE_VAR(*newidx)->state == 0)
            {
                SPRITE_VAR(*newidx)->state = 2;
                return;
            }
            
            ret = nextnum(newidx, oldidx);
        }
        while (ret);
    }
    
    createlem(newidx, oldidx);
}

void put(u16 idx)
{
    alis.fmuldes = 0;
    putin(idx);
}

void putin(u16 idx)
{
    // if there is no scene opened, there is nowhere to add sprite
    s16 scrnidx = get_scr_screen_id((get_0x16_screen_id(alis.script->vram_org)));
    if (scrnidx == 0)
        return;
    
    s16 x = image.depx;
    s16 z = image.depz;
    s16 y = image.depy;
    
    s32 addr = adresdes(idx);
    u8 *resourcedata = alis.mem + addr + xread32(addr);
    if (resourcedata[0] > 0x80)
    {
        if (resourcedata[0] == 0xfe)
        {
            topalette(resourcedata, 0);
            return;
        }
        
        // handle composit images
     
        s16 rsrccount = resourcedata[1];
        u8 *currsrc = resourcedata + 2;
        
        u8 invx;
        u8 muldes;

        for (s32 i = 0; i < rsrccount; i++)
        {
            invx = image.invert_x;
            muldes = alis.fmuldes;
            
            s16 curelem = read16(currsrc + 0);
            s16 curdepx = read16(currsrc + 2);
            if (*((u8 *)(&image.invert_x)) != 0)
                curdepx = -curdepx;

            image.depx += curdepx;

            s16 curdepy = read16(currsrc + 4);
            image.depy = curdepy + y;

            s16 curdepz = read16(currsrc + 6);
            image.depz = curdepz + z;
            
            if (curelem < 0)
            {
                curelem = curelem & 0x7fff;
                *((u8 *)(&image.invert_x)) ^= 1;
            }
            
            alis.fmuldes = 1;

            putin(curelem);

            image.depx = x;
            image.depy = y;
            image.depz = z;
            
            image.invert_x = invx;
            alis.fmuldes = muldes;

            currsrc += 8;
        }
        
        alis.fmuldes = 0;
    }
    else
    {
        u16 newidx = 0;
        u16 oldidx = 0;
        
        if (alis.fadddes == 0)
        {
            if (alis.fmuldes)
            {
                getelem(&newidx, &oldidx);
            }
            else
            {
                if (searchelem(&newidx, &oldidx) == 0)
                    createlem(&newidx, &oldidx);
                
                if (-1 < SPRITE_VAR(newidx)->state)
                    SPRITE_VAR(newidx)->state = 2;
            }
        }
        else
        {
            getelem(&newidx, &oldidx);
        }

        addr = adresdes(idx);

        sSprite *cursprvar = SPRITE_VAR(newidx);
        cursprvar->data       = addr;
        cursprvar->newad      = 0;
        cursprvar->flaginvx   = (u8)image.invert_x;
        cursprvar->depx       = image.oldcx + image.depx;
        cursprvar->depy       = image.oldcy + image.depy;
        cursprvar->depz       = image.oldcz + image.depz;
        cursprvar->credon_off = get_0x25_credon_credoff(alis.script->vram_org);
        cursprvar->creducing  = get_0x27_creducing(alis.script->vram_org);
        cursprvar->clinking   = get_0x2a_clinking(alis.script->vram_org);
        cursprvar->cordspr    = get_0x2b_cordspr(alis.script->vram_org);
        cursprvar->script_ent = get_0x0e_script_ent(alis.script->vram_org);
        
        s32 contextsize = get_context_size();
        if (contextsize > 0x2e)
        {
            cursprvar->chsprite = get_0x2f_chsprite(alis.script->vram_org);
        }
        else if (contextsize > 0x2e)
        {
            // TODO: ...
        }
        
        cursprvar->credon_off = get_0x25_credon_credoff(alis.script->vram_org);
        if (-1 < (s8)cursprvar->credon_off)
        {
            cursprvar->creducing = get_0x27_creducing(alis.script->vram_org);
            cursprvar->credon_off = get_0x26_creducing(alis.script->vram_org);
            if ((s8)cursprvar->credon_off < 0)
            {
                cursprvar->creducing = 0;
                cursprvar->credon_off = xread8(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 0x1f);
            }
        }
    }
    
    if (alis.fmuldes == 0)
    {
        u16 newidx = 0;
        u16 oldidx = 0;

        if (searchelem(&newidx, &oldidx) == 0)
            return;

        do
        {
            while (SPRITE_VAR(newidx)->state == 0)
            {
                killelem(&newidx, &oldidx);
                if (newidx == 0)
                {
                    alis.fadddes = 0;
                    return;
                }

                if (!testnum(&newidx))
                {
                    alis.fadddes = 0;
                    return;
                }
            }
        }
        while (nextnum(&newidx, &oldidx));
    }
    
    alis.fadddes = 0;
}

void put_char(s8 character)
{
    if (alis.charmode == 0)
    {
        // io_putchar(d0);
        return;
    }
    
    if (alis.charmode != 1)
    {
        if (alis.charmode != 2)
        {
            return;
        }
        
        if (character == '\n')
        {
            image.depz -= alis.fohaut;
        }
        else if (character == '\r')
        {
            image.depx = 0;
        }
        else
        {
            if (character != ' ')
            {
                if (alis.fonum < 0)
                {
                    return;
                }
                
                s16 charidx = ((s16)character) - alis.foasc;
                if ((-1 < charidx) && ((s16)(charidx - alis.fomax) < 0))
                {
                    alis.flagmain = 1;
                    image.invert_x = 0;
                    alis.fadddes = 1;
                    put(charidx + alis.fonum);
                }
            }
            
            image.depx += alis.folarg;
        }
    }
}

void put_string(void)
{
    for (char *strptr = alis.sd7; *strptr; strptr++)
    {
        put_char(*strptr);
    }
}

u32 tprintd0[] = { 0x3b9aca00, 0x05f5e100, 0x00989680, 0x000f4240, 0x000186a0, 0x00002710, 0x000003e8, 0x00000064, 0x0000000a, 0x00000001 };

void valtostr(char *string, s16 value)
{
    char *strptr = string;
    char *tmpptr;

    s32 tmpval = value;
    if (tmpval < 0)
    {
        strptr = string + 1;
        *string = 0x2d;
        tmpval = -tmpval;
    }
    
    s32 *tabptr = (int *)&tprintd0;
    s32 tprintv;
    s16 length = 9;
    s16 cw;
    char c;
    u8 res = 0;

    do
    {
        tprintv = *tabptr++;
        cw = -0x30;
        
        do
        {
            tmpval -= tprintv;
            if (tmpval < 0)
                break;
            
            cw --;
        }
        while (cw != -1);
        
        c = -(char)cw;
        if (c != 0x30)
        {
            res = 1;
            tmpptr = strptr + 1;
            *strptr = c;
        }
        else if (res)
        {
            tmpptr = strptr + 1;
            *strptr = c;
        }
        else
        {
            tmpptr = strptr;
        }
 
        tmpval += tprintv;
        length --;
        strptr = tmpptr;
        
        if (length == -1)
        {
            if (!res)
            {
                *strptr++ = c;
            }

            *strptr = 0;
            return;
        }
    }
    while (1);
}


#pragma mark -
#pragma mark calculate what to draw and where


s16 inilink(s16 elemidx)
{
    image.blocx1 = 0x7fff;
    image.blocy1 = 0x7fff;
    image.blocx2 = 0x8000;
    image.blocy2 = 0x8000;

    return SPRITE_VAR(elemidx)->clinking;
}

u8 calcfen(u16 elemidx1, u16 elemidx3)
{
    if (image.joints == 0)
    {
        sSprite *idx1sprvar = SPRITE_VAR(elemidx1);
        if (idx1sprvar->newd < 0)
            return 0;
        
        image.blocx1 = idx1sprvar->newx;
        image.blocy1 = idx1sprvar->newy;
        image.blocx2 = image.blocx1 + idx1sprvar->width + 1;
        image.blocy2 = image.blocy1 + idx1sprvar->height + 1;
    }
    
    sSprite *idx3sprvar = SPRITE_VAR(elemidx3);
    s16 tmpx = idx3sprvar->newx;
    s16 tmpy = idx3sprvar->newy;
    s16 tmpw = idx3sprvar->depx + 1;
    s16 tmph = idx3sprvar->depy + 1;
    
    if (image.blocx1 <= tmpw && image.blocy1 <= tmph && tmpx <= image.blocx2 && tmpy <= image.blocy2)
    {
        image.fenx1 = image.blocx1;
        if (image.blocx1 < tmpx)
            image.fenx1 = tmpx;
        
        image.feny1 = image.blocy1;
        if (image.blocy1 < tmpy)
            image.feny1 = tmpy;
        
        image.fenx2 = image.blocx2;
        if (tmpw < image.blocx2)
            image.fenx2 = tmpw;
        
        image.feny2 = image.blocy2;
        if (tmph < image.blocy2)
            image.feny2 = tmph;
        
        return 1;
    }
    
    return 0;
}

u8 clipfen(sSprite *sprite)
{
    image.fclip = 0;
    s16 spritex1 = sprite->newx;
    if (image.fenx2 < spritex1)
        return image.fclip;

    s16 spritey1 = sprite->newy;
    if (image.feny2 < spritey1)
        return image.fclip;

    s16 spritex2 = sprite->depx;
    if (spritex2 < image.fenx1)
        return image.fclip;

    s16 spritey2 = sprite->depy;
    if (spritey2 < image.feny1)
        return image.fclip;

    if (spritex1 < image.fenx1)
        spritex1 = image.fenx1;

    if (spritey1 < image.feny1)
        spritey1 = image.feny1;

    if (image.fenx2 < spritex2)
        spritex2 = image.fenx2;

    if (image.feny2 < spritey2)
        spritey2 = image.feny2;

    image.clipx1 = spritex1;
    image.clipy1 = spritey1;
    image.clipx2 = spritex2;
    image.clipy2 = spritey2;
    
    image.clipl = (image.clipx2 - image.clipx1) + 1;
    image.cliph = (image.clipy2 - image.clipy1) + 1;
    image.fclip = 1;
    return image.fclip;
}

u16 rangesprite(u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    sSprite *sprite1 = SPRITE_VAR(elemidx1);
    s16 newd1  =  sprite1->newd;
    s8 cordspr1 = sprite1->cordspr;
    s8 numelem1 = sprite1->numelem;
    if (image.wback != 0 && elemidx1 != image.backsprite && image.backprof <= newd1)
    {
        image.pback = 1;
    }

    sSprite *sprite3 = SPRITE_VAR(elemidx3);
    while (sprite3->link != 0)
    {
        sSprite *linksprite = SPRITE_VAR(sprite3->link);
        if (-1 < linksprite->state && linksprite->newd <= newd1 && (linksprite->newd < newd1 || (linksprite->cordspr <= cordspr1 && (linksprite->cordspr < cordspr1 || (linksprite->numelem <= numelem1 && (linksprite->numelem < numelem1 || -1 == sprite1->state))))))
        {
            break;
        }
        
        elemidx3 = sprite3->link;
        sprite3 = linksprite;
    }

    sprite1->state = 0;
    sprite1->link = sprite3->link;
    sprite3->link = elemidx1;

    return (elemidx2 == elemidx3) ? elemidx1 : elemidx2;
}

void tstjoints(u16 elemidx)
{
    sSprite *sprvar = SPRITE_VAR(elemidx);
    s16 tmpx = sprvar->newx + sprvar->width;
    s16 tmpy = sprvar->newy + sprvar->height;
    
    if (-1 < sprvar->newd && -1 < image.newd && sprvar->newx <= image.newx + image.newl && sprvar->newy <= image.newy + image.newh && image.newx <= tmpx && image.newy <= tmpy)
    {
        image.blocx1 = sprvar->newx;
        if (image.newx < sprvar->newx)
            image.blocx1 = image.newx;

        image.blocy1 = sprvar->newy;
        if (image.newy < sprvar->newy)
            image.blocy1 = image.newy;

        if (tmpx < (s16)(image.newx + image.newl))
            tmpx = image.newx + image.newl;

        if (tmpy < (s16)(image.newy + image.newh))
            tmpy = image.newy + image.newh;

        image.blocx2 = tmpx;
        image.blocy2 = tmpy;
        image.joints = 1;

        return;
    }
    
    image.joints = 0;
}

void scalaire(s16 scene, s16 *x, s16 *y, s16 *z)
{
    if (get_scr_numelem(scene) < 0)
    {
        s32 prevx = (get_scr_unknown0x20(scene) * *x + get_scr_unknown0x21(scene) * *y + get_scr_unknown0x22(scene) * *z);
        s32 prevy = (get_scr_unknown0x23(scene) * *x + get_scr_unknown0x24(scene) * *y + get_scr_unknown0x25(scene) * *z);
        s32 prevz = (get_scr_unknown0x26(scene) * *x + get_scr_unknown0x27(scene) * *y + get_scr_unknown0x28(scene) * *z);
        *x = prevx;
        *y = prevz;
        *z = prevy;
    }
    else
    {
        s16 prevy = *y;
        *y = *y * get_scr_unknown0x24(scene) - *z;
        *z = prevy;
    }
}

void depscreen(u16 scene, u16 elemidx)
{
    set_scr_depx(scene, get_scr_depx(scene) + get_scr_unknown0x2a(scene));
    set_scr_depy(scene, get_scr_depy(scene) + get_scr_unknown0x2c(scene));
    set_scr_depz(scene, get_scr_depz(scene) + get_scr_unknown0x2e(scene));
    
    for (sSprite *sprite = SPRITE_VAR(elemidx); sprite; sprite = SPRITE_VAR(sprite->link))
    {
        if (-1 < (s8)sprite->chsprite && sprite->state == 0)
            sprite->state = 2;
    }
    
    if ((alis.platform.uid == EGameTarghan0 || alis.platform.uid == EGameTarghan1) && (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAtari))
    {
        set_scr_state(scene, get_scr_state(scene) & 0x7f);
    }
}

void deptopix(u16 scene, u16 elemidx)
{
    if ((get_scr_numelem(scene) & 2) == 0)
    {
        sSprite *elemsprvar = SPRITE_VAR(elemidx);
        
        scene = elemsprvar->screen_id;
        image.newf = elemsprvar->flaginvx;
        s16 tmpdepx = elemsprvar->depx - get_scr_depx(scene);
        s16 tmpdepy = elemsprvar->depy - get_scr_depy(scene);
        s16 tmpdepz = elemsprvar->depz - get_scr_depz(scene);

        scalaire(scene, &tmpdepx, &tmpdepy, &tmpdepz);

        s16 offset = 0;
        u8 cred = get_scr_creducing(scene);
        if (-1 < (s8)cred && -1 < (s8)elemsprvar->credon_off)
        {
            if (tmpdepz == 0)
            {
                tmpdepz = 1;
            }
            
            tmpdepx = (s16)(((s32)tmpdepx << (cred & 0x3f)) / (s32)tmpdepz);
            tmpdepy = (s16)(((s32)tmpdepy << (cred & 0x3f)) / (s32)tmpdepz);
            offset = (tmpdepz >> (elemsprvar->credon_off & 0x3f)) + (s8)elemsprvar->creducing;
            if (offset < 0)
            {
                offset = 0;
            }
            else
            {
                int clink = (s8)xread8(alis.basemain + scene + 0x1e);
                if (clink < offset)
                {
                    offset = clink;
                }
                
                offset <<= 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0a(scene);
        tmpdepy = (tmpdepy >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0c(scene);

        image.newad = elemsprvar->data + offset;
        
        u8 *spritedata = (alis.mem + image.newad + xread32(image.newad));
        if (spritedata[0] == 3)
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4);
            tmpdepy += read16(spritedata + 6);
            if (spritedata[1] != 0)
            {
                image.newf ^= 1;
            }
            
            image.newad += (s16)(read16(spritedata + 2) << 2);
            spritedata = alis.mem + image.newad + xread32(image.newad);
        }
        
        image.newl = read16(spritedata + 2);
        image.newh = read16(spritedata + 4);
        image.newzoomx = 0;
        image.newzoomy = 0;
        
        
        // TODO:
//        if (alis.platform.version < 30)
//        {
//            u32 reducing = elemsprvar->calign2c;
//            if (reducing != 0)
//            {
//                if ((-1 < (char)elemsprvar->calign2d)
//                {
//                    u8 bVar3 = (u8)(offset >> 2)
//                    if (elemsprvar->calign2d < bVar3)
//                    {
//                        u8 bVar1 = get_scr_clinking(scene) - elemsprvar->calign2d;
//                        if ((-1 < (short)((ushort)bVar1 << 8)) && (bVar1 != 0))
//                        {
//                            reducing *= (reducing * (bVar3 - elemsprvar->calign2d)) / bVar1 & 0xffff;
//                        }
//                    }
//                }
//
//                tmpdepx = (tmpdepx >> (reducing & 0x3f)) << (reducing & 0x3f);
//            }
//        }
        
        if (alis.platform.kind == EPlatformMac)
        {
            mac_update_pos(&tmpdepx, &tmpdepy);
        }
        
        image.newx = tmpdepx - (image.newl >> 1);
        image.newy = tmpdepy - (image.newh >> 1);
        image.newd = tmpdepz;
    }
}

void waitphysic(void)
{
    do {} while (image.fphysic != 0);
}

void trsfen(u8 *src, u8 *tgt)
{
    if (image.fenx2 > alis.platform.width && image.feny2 > alis.platform.height)
    {
        return;
    }
    
    src += image.fenx1 + image.feny1 * alis.platform.width;
    tgt += image.fenx1 + image.feny1 * alis.platform.width;
    
    s16 skip = alis.platform.width - (image.fenx2 - image.fenx1);

    for (s32 y = image.feny1; y < image.feny2; y++)
    {
        for (s32 x = image.fenx1; x < image.fenx2; x++, src++, tgt++)
        {
            *tgt = *src;
        }
        
        tgt += skip;
        src += skip;
    }
}

void phytolog(void)
{
    image.fenx1 = 0;
    image.feny1 = 0;
    image.fenx2 = alis.platform.width - 1;
    image.feny2 = alis.platform.height - 1;
    trsfen(image.physic, image.logic);
}

void mouserase(void)
{
    // TODO: ...
//    u8 *paVar7 = physic;
//    if (fremouse2 != 0)
//    {
//        paVar7 = logic;
//    }
//
//    s16 sVar2 = 0;
//    s16 sVar4 = 0xf;
//    s16 sVar3 = 0x120;
//
//    if (-1 < savmouse[0])
//    {
//        u8 bVar1 = 0x12f < savmouse[0];
//        if (bVar1)
//        {
//            sVar2 = 0x10;
//            sVar3 = 0x130;
//        }
//
//        u32 *puVar5 = savmouse2;
//        if ((s16)(-savmouse[1] + 0xb8) < 0)
//        {
//            sVar4 = -savmouse[1] + 199;
//        }
//
//        u32 *puVar8 = (u32 *)(paVar7 + savmouse[1] * 0x140 + (savmouse[0] & 0xfff0));
//
//        do
//        {
//            u32 *puVar6;
//            u32 *puVar9;
//            if (!bVar1)
//            {
//                puVar8[0] = puVar5[0];
//                puVar8[1] = puVar5[1];
//                puVar6 = puVar5 + 3;
//                puVar9 = puVar8 + 3;
//                puVar8[2] = puVar5[2];
//                puVar5 = puVar5 + 4;
//                puVar8 = puVar8 + 4;
//                puVar9[0] = puVar6[0];
//            }
//
//            puVar8[0] = puVar5[0];
//            puVar8[1] = puVar5[1];
//            puVar8[2] = puVar5[2];
//            puVar8[3] = puVar5[3];
//            puVar8 = (u32 *)((s32)puVar8 + sVar3 + 0x10);
//            puVar5 = (u32 *)((s32)puVar5 + sVar2 + 0x10);
//            sVar4 --;
//        }
//        while (sVar4 != -1);
//    }
}

void tvtofen(void)
{
    // image.fenx2 = fenlargw * 4 + image.fenx1 - 1;
    trsfen(image.physic, image.logic);
}

void memfen(void)
{
    image.ptabfen[0] = image.fenx1;
    image.ptabfen[1] = image.fenx2;
    image.ptabfen[2] = image.feny1;
    image.ptabfen[3] = image.feny2;
    image.ptabfen += 4;

    s16 *endtabfen = image.tabfen + sizeof(image.tabfen);
    if (image.ptabfen < endtabfen)
    {
        return;
    }

    // error
    debug(EDebugError, "ptabfen pointing outside of available mem!\n");
}

void oldfen(void)
{
    for (s16 *tabptr = image.tabfen; tabptr < image.ptabfen; tabptr += 4)
    {
        image.fenx1 = tabptr[0];
        image.fenx2 = tabptr[1];
        image.feny1 = tabptr[2];
        image.feny2 = tabptr[3];
        tvtofen();
    }
}

void setphysic(void)
{
    if (image.insid == 0)
    {
        host.pixelbuf.data = image.physic;
    }

    image.bufpack = image.logic;
    image.fphysic = 1;
}

u8 *folscreen(u8 *scene)
{
    // TODO: ...
//    s16 *psVar1 = *(s16 **)(alis.atent + *(s16 *)(scene + 0x60));
//    if ((-1 < *(s16 *)(scene + 0x60)) && (psVar1 != (s16 *)0x0))
//    {
//        if ((scene[0x84] & 1) != 0)
//        {
//            if ((s16)(*psVar1 + *(s16 *)(scene + 0x86)) != *(s16 *)(scene + 0x16))
//            {
//                *(s16 *)(scene + 0x16) = *psVar1 + *(s16 *)(scene + 0x86);
//                *scene = *scene | 0x80;
//            }
//            if ((s16)(psVar1[4] + *(s16 *)(scene + 0x88)) != *(s16 *)(scene + 0x18))
//            {
//                *(s16 *)(scene + 0x18) = psVar1[4] + *(s16 *)(scene + 0x88);
//                *scene = *scene | 0x80;
//            }
//            if ((s16)(psVar1[8] + *(s16 *)(scene + 0x8a)) != *(s16 *)(scene + 0x1a))
//            {
//                *(s16 *)(scene + 0x1a) = psVar1[8] + *(s16 *)(scene + 0x8a);
//                *scene = *scene | 0x80;
//            }
//        }
//
//        if ((scene[0x84] & 2) != 0)
//        {
//            s16 sVar2 = psVar1[0x0c] + psVar1[0x20] + *(s16 *)(scene + 0x8c);
//            if (sVar2 != *(s16 *)(scene + 0x34))
//            {
//                *(s16 *)(scene + 0x34) = sVar2;
//                *scene = *scene | 0x80;
//            }
//            sVar2 = psVar1[0x10] + psVar1[0x22] + *(s16 *)(scene + 0x8e);
//            if (sVar2 != *(s16 *)(scene + 0x36))
//            {
//                *(s16 *)(scene + 0x36) = sVar2;
//                *scene = *scene | 0x80;
//            }
//            sVar2 = psVar1[0x14] + psVar1[0x24] + *(s16 *)(scene + 0x90);
//            if (sVar2 != *(s16 *)(scene + 0x38))
//            {
//                *(s16 *)(scene + 0x38) = sVar2;
//                *scene = *scene | 0x80;
//            }
//        }
//    }

    return scene;
}

void addlink(u16 elemidx)
{
    sSprite *elemsprvar = SPRITE_VAR(elemidx);
    if (-1 < elemsprvar->newd)
    {
        s16 tmp = elemsprvar->newx;
        if (tmp <= image.blocx1)
            image.blocx1 = tmp;

        tmp += elemsprvar->width + 1;
        if (image.blocx2 <= tmp)
            image.blocx2 = tmp;

        tmp = elemsprvar->newy;
        if (tmp <= image.blocy1)
            image.blocy1 = tmp;

        tmp += elemsprvar->height + 1;
        if (image.blocy2 <= tmp)
            image.blocy2 = tmp;
    }
}

u16 inouvlink(u16 scene, u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    deptopix(scene, elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    elem1sprvar->newad = image.newad;
    elem1sprvar->newx = image.newx;
    elem1sprvar->newy = image.newy;
    elem1sprvar->newd = image.newd;
    elem1sprvar->newf = image.newf;
    elem1sprvar->width = image.newl;
    elem1sprvar->height = image.newh;
    elem1sprvar->newzoomx = image.newzoomx;
    elem1sprvar->newzoomy = image.newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

u16 iremplink(u16 scene, u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    addlink(elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    if (image.wback != 0 && image.backprof <= elem1sprvar->newd)
    {
        image.pback = 1;
        return inouvlink(scene, elemidx1, elemidx2, elemidx3);
    }
    
    deptopix(scene, elemidx1);
    
    elem1sprvar->newad = image.newad;
    elem1sprvar->newx = image.newx;
    elem1sprvar->newy = image.newy;
    elem1sprvar->newd = image.newd;
    elem1sprvar->newf = image.newf;
    elem1sprvar->width = image.newl;
    elem1sprvar->height = image.newh;
    elem1sprvar->newzoomx = image.newzoomx;
    elem1sprvar->newzoomy = image.newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

u16 iefflink(u16 elemidx1, u16 elemidx2)
{
    addlink(elemidx1);

    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;
    elem1sprvar->to_next = image.libsprit;
    
    image.libsprit = elemidx1;
    if (image.wback != 0 && image.backprof <= elem1sprvar->newd)
    {
        image.pback = 1;
    }
    
    return elemidx2;
}

s16 scdirect = 0;
u8 fdoland = 0;

void clrfen(void)
{
    s16 tmpx = image.fenx2 - image.fenx1;
    for (s16 y = image.feny1; y < image.feny2; y++)
    {
        memset(image.physic + image.fenx1 + y * alis.platform.width, 0, tmpx);
    }
}

void clipback(void)
{
    if (image.clipy1 < ((s16 *)(&image.backx1))[1])
    {
        if (image.clipy2 < ((s16 *)(&image.backx1))[1])
        {
            return;
        }
    }
    else
    {
        if (image.clipy2 <= ((s16 *)(&image.backx2))[1])
        {
            image.cback = 1;
            return;
        }
        
        if (((s16 *)(&image.backx2))[1] < image.clipy1)
        {
            return;
        }
    }
    
    image.cback = -1;
}

void destofen(sSprite *sprite)
{
    if (sprite->newad == 0)
        return;
    
    u8 *bitmap = (alis.mem + sprite->newad + xread32(sprite->newad));
    if (*bitmap < 0)
        return;

    sRect pos = {
        .x1 = sprite->newx,
        .y1 = sprite->newy,
        .x2 = sprite->newx + (s16)read16(bitmap + 2),
        .y2 = sprite->newy + (s16)read16(bitmap + 4) };

    s8 flip = sprite->newf;
    s32 width = sprite->width + 1;
    s32 height = sprite->height + 1;

    if (image.clipx2 < pos.x1)
        return;

    if (image.clipy2 < pos.y1)
        return;

    if (pos.x2 < image.clipx1)
        return;

    if (pos.y2 < image.clipy1)
        return;

    image.blocx1 = pos.x1;
    if (pos.x1 < image.clipx1)
        image.blocx1 = image.clipx1;

    image.blocy1 = pos.y1;
    if (pos.y1 < image.clipy1)
        image.blocy1 = image.clipy1;

    image.blocx2 = pos.x2;
    if (image.clipx2 < pos.x2)
        image.blocx2 = image.clipx2;

    image.blocy2 = pos.y2;
    if (image.clipy2 < pos.y2)
        image.blocy2 = image.clipy2;

    u8 *at = bitmap + 6;

    sRect bmp = {
        .x1 = 0,
        .y1 = 0,
        .x2 = width,
        .y2 = height };

    if (image.blocx1 < 0)
    {
        bmp.x1 -= image.blocx1;
        bmp.x2 += image.blocx1;
    }
    
    if (image.blocx1 > pos.x1)
    {
        bmp.x1 += (image.blocx1 - pos.x1);
        bmp.x2 -= (image.blocx1 - pos.x1);
    }
    
    if (image.blocx2 <= pos.x2)
    {
        bmp.x2 -= (pos.x2 - image.blocx2);
    }

    if (image.blocy1 > pos.y1)
    {
        bmp.y1 += (image.blocy1 - pos.y1);
        bmp.y2 -= (image.blocy1 - pos.y1);
    }

    if (image.blocy2 < pos.y2)
    {
        bmp.y2 -= (pos.y2 - image.blocy2);
    }
    
    switch (bitmap[0])
    {
        case 0x01:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_rect(&pos, &bmp, bitmap[1]);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
            {
                draw_cga_rect(&pos, &bmp, bitmap[1]);
            }
            else
            {
                draw_rect(&pos, &bmp, bitmap[1]);
            }
            break;
        }
            
        case 0x00:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_mono_0(at, &pos, &bmp, width, flip);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
            {
                draw_dos_cga_0(at, &pos, &bmp, width, flip);
            }
            else
            {
                draw_st_4bit_0(at, &pos, &bmp, width, flip);
            }

            break;
        }
            
        case 0x02:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_mono_0(at, &pos, &bmp, width, flip);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.uid == EGameMadShow)
            {
                draw_dos_cga_2(at, &pos, &bmp, width, flip);
            }
            else
            {
                draw_st_4bit_2(at, &pos, &bmp, width, flip);
            }

            break;
        }
            
        case 0x10:
        {
            if (alis.platform.px_format == EPxFormatAmPlanar)
            {
                draw_ami_5bit_2(at, &pos, &bmp, width, height, flip);
            }
            else
            {
                draw_4to8bit_0(at, &pos, &bmp, width, flip);
            }

            break;
        }

        case 0x12:
        {
            if (alis.platform.px_format == EPxFormatAmPlanar)
            {
                draw_ami_5bit_2(at, &pos, &bmp, width, height, flip);
            }
            else
            {
                draw_4to8bit_2(at, &pos, &bmp, width, flip);
            }
            break;
        }
            
        case 0x14:
        {
            draw_8bit_0(at, &pos, &bmp, width, flip);
            break;
        }

        case 0x16:
        {
            draw_8bit_2(at, &pos, &bmp, width, flip);
            break;
        }
            
        case 0x40:
        {
             draw_fli_video(bitmap);
            break;
        }
            
        case 0x7f:
        {
            draw_map(sprite, sprite->newad + xread32(sprite->newad), (sRect){ .x1 = (bmp.x1 + pos.x1), .y1 = (bmp.y1 + pos.y1), .x2 = (bmp.x1 + bmp.x2 + pos.x1), .y2 = (bmp.y1 + bmp.y2 + pos.y1) });
            break;
        }
            
        default:
            debug(EDebugError, "UNKNOWN RESOURCE TYPE: %d", bitmap[0]);
    }
}

void calctop(void)
{
    return;
}

void fentotv(void)
{
    trsfen(image.logic, image.physic);
}

void fenetre(u16 scene, u16 elemidx1, u16 elemidx3)
{
    u16 tmpidx;

    sSprite *sprite;

    if ((get_scr_state(scene) & 0x40) != 0)
    {
        return;
    }

    scdirect = 0;
    if (calcfen(elemidx1, elemidx3))
    {
        if ((get_scr_numelem(scene) & 2) != 0)
        {
            fdoland = 1;
        }
        
        for (s16 scridx = screen.ptscreen; scridx != 0; scridx = get_scr_to_next(scridx))
        {
            if ((get_scr_state(scridx) & 0x40) == 0)
            {
                tmpidx = get_scr_screen_id(scridx);
                if (tmpidx != 0)
                {
                    clipfen(SPRITE_VAR(tmpidx));
                    
                    if (image.fclip != 0)
                    {
                        if ((get_scr_numelem(scridx) & 2) == 0)
                        {
                            image.cback = 0;
                            if ((get_scr_numelem(scridx) & 4) != 0)
                            {
                                clipback();
                            }
                            
                            if ((get_scr_numelem(scridx) & 0x40) == 0)
                            {
                                clrfen();
                            }
                            
                            if (image.cback)
                            {
                                if (image.cback < 0)
                                {
                                    if ((image.wback != 0) && (image.pback != 0))
                                    {
                                        image.wlogic = image.backmap;
                                        image.wlogx1 = image.backx1;
                                        image.wlogx2 = image.backx2;
                                        image.wlogy1 = image.backy1;
                                        image.wlogy2 = image.backy2;
                                        image.wloglarg = image.backlarg;
                                        
                                        if (image.clipy1 <= image.backx1)
                                            image.clipy1 = image.backx1;
                                        
                                        if (image.backx2 <= image.clipy2)
                                            image.clipy2 = image.backx2;
                                        
                                        sprite = SPRITE_VAR(tmpidx);
                                        while (true)
                                        {
                                            tmpidx = sprite->link;
                                            if ((tmpidx == 0) || (tmpidx == image.backsprite))
                                                break;
                                            
                                            sprite = SPRITE_VAR(tmpidx);
                                            if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                            {
                                                destofen(sprite);
                                            }
                                        }
                                        
                                        image.wlogic = image.logic;
                                        image.wlogx1 = image.logx1;
                                        image.wlogx2 = image.logx2;
                                        image.wlogy1 = image.logy1;
                                        image.wlogy2 = image.logy2;
                                        image.wloglarg = image.loglarg;
                                    }
                                }
                                else
                                {
                                    if (image.pback == 0)
                                        goto fenetre31;
                                    
                                    image.wlogic = image.backmap;
                                    image.wlogx1 = image.backx1;
                                    image.wlogx2 = image.backx2;
                                    image.wlogy1 = image.backy1;
                                    image.wlogy2 = image.backy2;
                                    image.wloglarg = image.backlarg;
                                }
                            }
                            
                            sprite = SPRITE_VAR(tmpidx);
                            while ((tmpidx = sprite->link))
                            {
                                if (image.sback != 0 && tmpidx == image.backsprite)
                                {
                                    image.wlogic = image.logic;
                                    image.wlogx1 = image.logx1;
                                    image.wlogx2 = image.logx2;
                                    image.wlogy1 = image.logy1;
                                    image.wlogy2 = image.logy2;
                                    image.wloglarg = image.loglarg;
                                }
                                
                            fenetre31:
                                
                                sprite = SPRITE_VAR(tmpidx);
                                if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                {
                                    destofen(sprite);
                                    image.switchgo = 1;
                                }
                            }
                        }
                        else
                        {
                            calctop();
                        }
                    }
                }
            }
        }
        
//        // TODO: mouse
//        cVar2 = fremouse;
//        if (((-1 < fmouse) && (fmouse != 2)) && (alis.fswitch == 0))
//        {
//            do
//            {
//                fremouse = cVar2 + 1;
//            }
//            while (fremouse != 0);
//
//            if (tvmode == 0)
//            {
//                mousefen((s32)d0);
//            }
//            else
//            {
//                mouserase();
//            }
//        }
        
        if ((get_scr_numelem(scene) & 0x20) == 0)
        {
            if (alis.fswitch != 0)
            {
                if (image.wpag == 0)
                {
                    memfen();
                }
                
                return;
            }
            
            if (image.wpag == 0)
            {
                fentotv();
            }
        }
        
//        // TODO: mouse
//        if (alis.fswitch == 0)
//        {
//            if ((tvmode != 0) && (-1 < fmouse))
//            {
//                mouseput();
//            }
//
//            fremouse = -1;
//        }
    }
}

void scrolpage(void)
{
//    int *piVar1;
//    short sVar2;
//
//    paglogic = (int *)0x0;
//    pagphysic = addr_physic_vram_start;
//    piVar1 = io_malloc();
//    if (piVar1 != (int *)0x0)
//    {
//        pagphysic = (int *)(((int)piVar1 - 1U | 0xff) + 1);
//        paglogic = piVar1;
//        transfen((byte *)addr_physic_vram_start,(byte *)pagphysic);
//    }
//
//    vtiming = 0;
//    pagcalc();
//
//    if (image.pagdx != 0)
//    {
//        hbar_calc();
//    }
//    if (image.pagdy != 0)
//    {
//        vbar_calc();
//    }
//
//    sVar2 = image.pagcount - 1;
//    if (-1 < sVar2)
//    {
//        do
//        {
//            pagscroll();
//            if (image.pagdx != 0)
//            {
//                hpagbar();
//            }
//
//            if (image.pagdy != 0)
//            {
//                vpagbar();
//            }
//
//            piVar1 = pagphysic;
//            if (paglogic != (int *)0x0)
//            {
//                pagphysic = addr_physic_vram_start;
//                addr_physic_vram_start = piVar1;
//                setphysic();
//                waitphysic();
//            }
//            sVar2 = sVar2 - 1;
//        }
//        while (sVar2 != -1);
//
//        if (paglogic != (int *)0x0)
//        {
//            if (paglogic == addr_physic_vram_start)
//            {
//                transfen((byte *)addr_physic_vram_start,(byte *)pagphysic);
//                addr_physic_vram_start = pagphysic;
//                setphysic();
//                waitphysic();
//            }
//
//            io_mfree();
//        }
//    }
//
//    if (pagcount != 0x14)
//    {
//        transfen((byte *)addr_physic_vram_start,logic);
//    }
}

u16 suitlin1(u16 a2, u16 d1w, u16 d2w, u16 d3w, u16 d4w)
{
    while ((void)(d2w = d1w), (d1w = SPRITE_VAR(d2w)->link) != 0)
    {
        if (d4w == SPRITE_VAR(d1w)->clinking)
        {
            s8 state = SPRITE_VAR(d1w)->state;
            if (state != 0)
            {
                if (state < 0)
                {
                    d1w = inouvlink(a2, d1w, d2w, d3w);
                }
                else if (state == 2)
                {
                    d1w = iremplink(a2, d1w, d2w, d3w);
                }
                else
                {
                    d1w = iefflink(d1w, d2w);
                }
            }
        }
    }

    image.joints = 1;
    return d1w;
}

void affiscr(u16 scene, u16 screenidx)
{
    u16 spriteidx = screenidx;
    u16 prevspidx;
    u16 linkidx;
    
//    if (a2[0x84] != 0)
//    {
//        a2 = folscreen(a2);
//    }
    
    if ((image.fremap != 0) || ((s8)get_scr_state(scene) < 0))
    {
        depscreen(scene, screenidx);
    }
    
    u8 draw = alis.platform.version == 0
                    ? alis.fswitch == 0
                    : ((get_scr_numelem(scene) & 2) == 0 || (get_scr_state(scene) & 0x80) == 0);
    
    if (draw)
    {
        image.wback = (get_scr_numelem(scene) & 4) != 0;
        image.wpag = 0;

        if ((get_scr_state(scene) & 0x20) != 0)
        {
            image.wpag = 1;
            image.spag --;
            if (image.spag == 0)
            {
                image.wpag = -1;
            }
        }
        
        if ((get_scr_numelem(scene) & 0x10) == 0)
        {
            while ((void)(prevspidx = spriteidx), (spriteidx = SPRITE_VAR(spriteidx)->link) != 0)
            {
                s8 state = SPRITE_VAR(spriteidx)->state;
                if (state != 0)
                {
                    image.joints = 0;
                    image.pback = 0;
                    if (state == 2)
                    {
                        linkidx = inilink(spriteidx);
                        if (linkidx >= 0)
                        {
                            spriteidx = iremplink(scene, spriteidx, prevspidx, screenidx);
                            suitlin1(scene, spriteidx, prevspidx, screenidx, linkidx);
                        }
                        else
                        {
                            sSprite *sprite = SPRITE_VAR(spriteidx);

                            u8 isback = image.wback == 0;
                            if (!isback)
                            {
                                isback = image.backprof == sprite->newd;
                                if (image.backprof <= sprite->newd)
                                {
                                    image.pback = 1;
                                    isback = false;
                                }
                            }
                            
                            deptopix(scene, spriteidx);
                            tstjoints(spriteidx);
                            
                            if (image.joints == 0)
                            {
                                SPRITE_VAR(prevspidx)->link = sprite->link;
                                fenetre(scene, spriteidx, screenidx);
                            }
                            else
                            {
                                SPRITE_VAR(prevspidx)->link = sprite->link;
                            }
                            
                            sprite->newad = image.newad;
                            sprite->newx = image.newx;
                            sprite->newy = image.newy;
                            sprite->newd = image.newd;
                            sprite->newf = image.newf;
                            sprite->width = image.newl;
                            sprite->height = image.newh;
                            sprite->newzoomx = image.newzoomx;
                            sprite->newzoomy = image.newzoomy;
                            
                            prevspidx = rangesprite(spriteidx, prevspidx, screenidx);
                        }
                    }
                    else
                    {
                        linkidx = inilink(spriteidx);
                        
                        if (state == -1)
                        {
                            spriteidx = inouvlink(scene, spriteidx, prevspidx, screenidx);
                        }
                        else
                        {
                            spriteidx = iefflink(spriteidx, prevspidx);
                        }
                        
                        if (linkidx < 0)
                        {
                            image.joints = 1;
                        }
                        else
                        {
                            suitlin1(scene, spriteidx, prevspidx, screenidx, linkidx);
                        }
                    }
                    
                    fenetre(scene, spriteidx, screenidx);
                    spriteidx = prevspidx;
                }
            }
        }
        else
        {
            image.fenx1 = get_scr_newx(scene);
            image.feny1 = get_scr_newy(scene);
            image.fenx2 = image.fenx1 + get_scr_width(scene);
            image.feny2 = image.feny1 + get_scr_height(scene);
            image.clipl = (image.fenx2 - image.fenx1) + 1;
            image.cliph = (image.feny2 - image.feny1) + 1;
            
            image.clipx1 = image.fenx1;
            image.clipy1 = image.feny1;
            image.clipx2 = image.fenx2;
            image.clipy2 = image.feny2;
            
            if ((get_scr_numelem(scene) & 0x40) == 0)
            {
                clrfen();
            }
            
            while ((void)(prevspidx = spriteidx), (spriteidx = SPRITE_VAR(spriteidx)->link) != 0)
            {
                sSprite *psprite = SPRITE_VAR(prevspidx);
                sSprite *csprite = SPRITE_VAR(spriteidx);
                if (csprite->state != 0)
                {
                    if (csprite->state == 1)
                    {
                        psprite->link = csprite->link;
                        csprite->to_next = image.libsprit;
                        image.libsprit = spriteidx;
                        spriteidx = prevspidx;
                    }
                    else
                    {
                        deptopix(scene, spriteidx);
                        psprite->link = csprite->link;
                        csprite->newad = image.newad;
                        csprite->newx = image.newx;
                        csprite->newy = image.newy;
                        csprite->newd = image.newd;
                        csprite->newf = image.newf;
                        csprite->width = image.newl;
                        csprite->height = image.newh;
                        csprite->newzoomx = image.newzoomx;
                        csprite->newzoomy = image.newzoomy;
                        spriteidx = rangesprite(spriteidx, prevspidx, screenidx);
                    }
                }
            }
            
            for (spriteidx = screen.ptscreen; spriteidx; spriteidx = get_scr_to_next(spriteidx))
            {
                if (((get_scr_state(spriteidx) & 0x40) == 0) && ((prevspidx = get_scr_screen_id(spriteidx)) != 0))
                {
                    clipfen(SPRITE_VAR(prevspidx));
                    
                    if (image.fclip != 0)
                    {
                        while ((prevspidx = SPRITE_VAR(prevspidx)->link) != 0)
                        {
                            if (-1 < SPRITE_VAR(prevspidx)->state && -1 < SPRITE_VAR(prevspidx)->newf && -1 < SPRITE_VAR(prevspidx)->newd)
                            {
                                destofen(SPRITE_VAR(prevspidx));
                                image.switchgo = 1;
                            }
                        }
                    }
                }
            }

            if ((alis.fswitch == 0) && (image.wpag == 0))
            {
                fentotv();
            }
        }
    }
    else
    {
        for (spriteidx = get_scr_to_next(scene); spriteidx; spriteidx = get_scr_to_next(spriteidx))
        {
            if (((get_scr_state(spriteidx) & 0x40) == 0) && ((prevspidx = get_scr_screen_id(spriteidx)) != 0))
            {
                clipfen(SPRITE_VAR(prevspidx));
                
                if (image.fclip != 0)
                {
                    while ((prevspidx = SPRITE_VAR(prevspidx)->link) != 0)
                    {
                        if (-1 < SPRITE_VAR(prevspidx)->state && -1 < SPRITE_VAR(prevspidx)->newf && -1 < SPRITE_VAR(prevspidx)->newd)
                        {
                            destofen(SPRITE_VAR(prevspidx));
                            image.switchgo = 1;
                        }
                    }
                }
            }
        }

        if ((alis.fswitch == 0) && (image.wpag == 0))
        {
            fentotv();
        }
    }
    
    if (image.wpag < 0)
    {
        set_scr_state(scene, get_scr_state(scene) & 0xdf);
        image.fenx1 = get_scr_newx(scene);
        image.feny1 = get_scr_newy(scene);
        image.fenx2 = get_scr_newx(scene) + get_scr_width(scene);
        image.feny2 = get_scr_newy(scene) + get_scr_height(scene);
        scrolpage();
    }
    
    // TODO: fix this hack
    // following 'if ()' is needed to draw rails under train on minimap in transartica
    if (alis.platform.uid != EGameTransartica)
    {
        set_scr_state(scene, get_scr_state(scene) & 0x7f);
    }
}

u32 itroutine(u32 interval, void *param)
{
    u8 prevtiming = image.vtiming;
    alis.timeclock ++;
    image.fitroutine = 1;
    image.vtiming ++;
    if (image.vtiming == 0) {
        image.vtiming = prevtiming;
    }
    
    if (image.palc != 0 && (--image.palt) == 0)
    {
        image.palt = image.palt0;
        topalet();
        image.palc --;
    }
    
    // TODO: audio
//    if ((bcanal0 != 0) && (-1 < bcanal0))
//    {
//        canal();
//    }
//
//    if ((bcanal1 != 0) && (-1 < bcanal1))
//    {
//        canal();
//    }
//
//    if ((bcanal2 != 0) && (-1 < bcanal2))
//    {
//        canal();
//        check3();
//    }
//
//    if ((bcanal3 != 0) && (-1 < bcanal3))
//    {
//        canal();
//    }
    
    image.fitroutine = 0;
    return 20;
}

void waitframe(void) {
    
    struct timeval now;
    while ((void)(gettimeofday(&now, NULL)), ((now.tv_sec * 1000000 + now.tv_usec) - (alis.time.tv_sec * 1000000 + alis.time.tv_usec) < FRAME_TICKS * alis.ctiming)) {
        usleep(1000);
    }

    alis.time = now;
}

void draw(void)
{
    VERIFYINTEGRITY;
    
    waitframe();
    
//    waitphysic();
    if ((alis.fswitch != 0) && (image.fphytolog != 0))
    {
        image.fphytolog = 0;
        phytolog();
    }
    
    image.vtiming = 0;
    
    if (alis.fswitch != 0)
    {
        // TODO: mouse
        if (image.fmouse < 0)
        {
            oldfen();
        }
        else
        {
            oldfen();
        }

        image.ptabfen = image.tabfen;
    }
    
    image.switchgo = 0;
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogx2 = image.logx2;
    image.wlogy1 = image.logy1;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
    
    s16 scnidx = screen.ptscreen;
    u8 *oldphys = image.physic;
    
    while (scnidx != 0)
    {
        if ((get_scr_state(scnidx) & 0x40) == 0)
        {
            affiscr(scnidx, get_scr_screen_id(scnidx));
        }

        scnidx = get_scr_to_next(scnidx);
    }

    image.fremap = 0;
    if (alis.fswitch != 0)
    {
        image.physic = image.logic;
        
        // NOTE: targhan doesn't use double buffering
        if (alis.platform.dbl_buf)
        {
            image.logic = oldphys;
        }
        
        setphysic();
    }
    
    sys_render(host.pixelbuf);

    VERIFYINTEGRITY;
}

// TODO: move to script.c
s16 debprotf(s16 target_id)
{
    s16 current_id;
    
    int start = 0;
    int mid;
    int end = alis.nbprog - 1;
    
    do
    {
        mid = (end + start) >> 1;
        current_id = read16((alis.mem + alis.atprog_ptr[mid]));
        if (current_id == target_id)
        {
            return mid;
        }

        if (current_id < target_id)
        {
            start = mid + 1;
        }
        else
        {
            end = mid - 1;
        }
    }
    while (start <= end);
    
    return -1;
}


#pragma mark -
#pragma mark Draw functions


void draw_mac_rect(sRect *pos, sRect *bmp, u8 color)
{
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            *tgt = color == 15 ? (w + h) % 2 : !color;
        }
    }
}

void draw_cga_rect(sRect *pos, sRect *bmp, u8 color)
{
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            *tgt = color % 4;
        }
    }
}

void draw_rect(sRect *pos, sRect *bmp, u8 color)
{
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            *tgt = color;
        }
    }
}

void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // Macintosh image
    
    u8 index, color;
    s16 wh;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 4;

            index = flip ? 3 - w % 4 : w % 4;
            color = *(at + wh + h * (width / 4));
            color = (color & masks[index]) >> rots[index];
            if (!(color & 2))
            {
                *tgt = !(color & 1);
            }
        }
    }
}

void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // Macintosh image
    
    u8 index, color;
    s16 wh;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 4;

            index = flip ? 3 - w % 4 : w % 4;
            color = *(at + wh + h * (width / 4));
            color = (color & masks[index]) >> rots[index];
            *tgt = !(color & 1);
        }
    }
}

void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index, opacity, color;
    s16 wh;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = ((flip ? (width - (w + 1)) : w) / 4) * 2;
            opacity = *(at + wh + h * (width / 2));

            index = flip ? 3 - w % 4 : w % 4;
            opacity = (opacity & masks[index]) >> rots[index];
            if (!opacity)
            {
                color = *(at + wh + 1 + h * (width / 2));
                color = (color & masks[index]) >> rots[index];
                *tgt = color;
            }
        }
    }
}

void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index, color;
    s16 wh;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = ((flip ? (width - (w + 1)) : w) / 4);

            index = flip ? 3 - w % 4 : w % 4;
            color = *(at + wh + h * (width / 4));
            color = (color & masks[index]) >> rots[index];
            *tgt = color;
        }
    }
}

void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // ST image
    
    // NOTE: values for ST ishar 3
    // palidx = sprite->screen_id != 82 && image.fdarkpal ? 128 : 0;
    
    u8 color;
    s16 wh;

    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 2;
            color = *(at + wh + h * (width / 2));
            color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
            if (color != 0)
            {
                *tgt = color;
            }
        }
    }
}

void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // ST image
    
    // NOTE: values for ST ishar 3
    // palidx = sprite->screen_id != 82 && image.fdarkpal ? 128 : 0;
    
    u8 color;
    s16 wh;

    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 2;
            color = *(at + wh + h * (width / 2));
            *tgt = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
        }
    }
}

void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    // 5 bit image
    
    u32 planesize = (width * height) / 8;
    
    u8 color, c0, c1, c2, c3, c4;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            s16 wh = (flip ? (width - (w + 1)) : w);
            s32 idx = (wh + h * width) / 8;
            c0 = *(at + idx);
            c1 = *(at + (idx += planesize));
            c2 = *(at + (idx += planesize));
            c3 = *(at + (idx += planesize));
            c4 = *(at + (idx += planesize));
            
            int bit = 7 - (wh % 8);
            
            color = ((c0 >> bit) & 1) | ((c1 >> bit) & 1) << 1 | ((c2 >> bit) & 1) << 2 | ((c3 >> bit) & 1) << 3 | ((c4 >> bit) & 1) << 4;
            if (color != 0)
                *tgt = color;
        }
    }
}

void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    // 5 bit image
    
    u32 planesize = (width * height) / 8;
    
    u8 c0, c1, c2, c3, c4;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            s16 wh = (flip ? (width - (w + 1)) : w);
            s32 idx = (wh + h * width) / 8;
            c0 = *(at + idx);
            c1 = *(at + (idx += planesize));
            c2 = *(at + (idx += planesize));
            c3 = *(at + (idx += planesize));
            c4 = *(at + (idx += planesize));
            
            int bit = 7 - (wh % 8);
            
            *tgt = ((c0 >> bit) & 1) | ((c1 >> bit) & 1) << 1 | ((c2 >> bit) & 1) << 2 | ((c3 >> bit) & 1) << 3 | ((c4 >> bit) & 1) << 4;
        }
    }
}

void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // 4 bit image
    
    u8 color;
    s16 wh;

    u8 palidx = at[0];
    at += 2;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 2;
            color = *(at + wh + h * (width / 2));
            color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
            if (color != 0)
                *tgt = palidx + color;
        }
    }
}

void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // 4 bit image
    
    u8 color;
    s16 wh;

    u8 palidx = at[0];
    at += 2;
    
    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            wh = (flip ? (width - (w + 1)) : w) / 2;
            color = *(at + wh + h * (width / 2));
            *tgt = palidx + (w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111));
        }
    }
}

void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // 8 bit image
    
    u8 color;
    
    at += 2;

    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            color = *(at + (flip ? width - (w + 1) : w) + h * width);
            if (color != 0)
                *tgt = color;
        }
    }
}

void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    // 8 bit image
    
    at += 2;

    for (s32 h = bmp->y1; h < bmp->y1 + bmp->y2; h++)
    {
        u8 *tgt = image.logic + (bmp->x1 + pos->x1) + ((pos->y1 + h) * host.pixelbuf.w);
        for (s32 w = bmp->x1; w < bmp->x1 + bmp->x2; w++, tgt++)
        {
            *tgt = *(at + (flip ? width - (w + 1) : w) + h * width);
        }
    }
}

void draw_fli_video(u8 *bitmap)
{
    // FLI video
    
    uint32_t size1 = read32(bitmap + 2);
    s8 *fliname = (s8 *)&bitmap[6];
    debug(EDebugVerbose, "FLI video (%s) %d bytes [", fliname, size1);

    uint32_t size2 = (*(uint32_t *)(&bitmap[32]));
    uint16_t frames = (*(uint16_t *)(&bitmap[38]));

    debug(EDebugVerbose, "size: %d frames: %d]\n", size2, frames);

    // TODO: ...
}

void draw_map(sSprite *sprite, u32 mapaddr, sRect lim)
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
        
        s32 addr = get_0x14_script_org_offset(vram);
        vram = xread32(addr + 0xe) + addr;
        
        addr = xread32(vram) + vram;
        u16 tempy = (image.blocy1 - sprite->newy) + xread16(mapaddr - 6);
        
        s32 yc = tempy / (u16)tiley;
        s16 yo = image.blocy1 - tempy % (u16)tiley;

        u32 tileaddr = mapaddr + 0x28;
        s16 t1 = image.blocx1 - sprite->newx;
        s16 t2 = xread16(mapaddr - 10);
        
        tileaddr += (tileoffset * ((u16)(t1 + t2) / tilex) + yc);
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
            u8 color = 63;
            
            for (s32 h = lim.y1; h < lim.y2; h++)
            {
                u8 *tgt = image.logic + lim.x1 + ((lim.y1 + h) * host.pixelbuf.w);
                for (s32 w = lim.x1; w < lim.x2; w++, tgt++)
                {
                    *tgt = color;
                }
            }
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
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    if ((w + posx1) < lim.x1)
                                        continue;

                                    if ((w + posx1) >= lim.x2)
                                        continue;

                                    if ((h + posy1) < lim.y1)
                                        continue;

                                    if ((h + posy1) >= lim.y2)
                                        continue;

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
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;

                                        if ((w + posx1) >= lim.x2)
                                            continue;

                                        if ((h + posy1) < lim.y1)
                                            continue;

                                        if ((h + posy1) >= lim.y2)
                                            continue;

                                        // tgt = image.logic + (w + posx1) + ((posy1 + h) * host.pixelbuf.w);
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
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;

                                        if ((w + posx1) >= lim.x2)
                                            continue;

                                        if ((h + posy1) < lim.y1)
                                            continue;

                                        if ((h + posy1) >= lim.y2)
                                            continue;

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
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    color = *(at + (flip ? width - (w + 1) : w) + h * width);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;
                                        
                                        if ((w + posx1) >= lim.x2)
                                            continue;
                                        
                                        if ((h + posy1) < lim.y1)
                                            continue;
                                        
                                        if ((h + posy1) >= lim.y2)
                                            continue;
                                        
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


#pragma mark -
#pragma mark Helper functions


void mac_update_pos(short *x,short *y)
{
    *x *= 1.5;
    *y *= 1.5;
}
