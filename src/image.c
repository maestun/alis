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


#ifndef max
# define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define FRAME_TICKS (1000000 / 50) // 25 fps looks about right in logo animation

#define DEBUG_CHECK 0

#if DEBUG_CHECK > 0
# define VERIFYINTEGRITY verifyintegrity()
#else
# define VERIFYINTEGRITY
#endif

sImage image = {
    .numelem = 0,
    .invert_x = 0,
    .depx = 0,
    .depy = 0,
    .depz = 0,
    .oldcx = 0,
    .oldcy = 0,
    .oldcz = 0
};

u8 tvmode = 0;

u8 svpalet[1024];
u8 svpalet2[1024];
u8 tpalet[1024];
u8 mpalet[1024];
float dpalet[1024];

u8 *dkpalet = 0;

u8 palc = 0;
u8 palt = 0;
u8 palt0 = 0;
u8 ftopal = 0;
u8 fdarkpal = 0;
u8 thepalet = 0;
u8 defpalet = 0;

s16 *ptabfen = 0;
s16 tabfen[640];

s32 bufrvb;

s32 mousflag;

s16 oldacx;
s16 oldacy;
s16 oldacz;

s8 nmode = 0;

u8 *bufpack = 0;

u8 timing = 0;
s8 fmouse = 0xff;
s16 fonum = 0xffff;
u8 sback = 0;

u32 newad;
s16 newx;
s16 newy;
s16 newd;
s16 newf;
s16 newh;
s16 newl;
s16 newzoomx;
s16 newzoomy;
s16 blocx1;
s16 blocy1;
s16 blocx2;
s16 blocy2;
u8 joints;
s16 fenx1;
s16 feny1;
s16 fenx2;
s16 feny2;
u8 sback;
u8 wback;
s8 cback;
u8 pback;
s16 backprof = 0;
s16 clipx1;
s16 clipy1;
s16 clipx2;
s16 clipy2;
s16 clipl;
s16 cliph;
u8 fclip;

u8 buffer0[320*200];
u8 buffer1[320*200];

u8 *physic = buffer0;
u8 *logic = buffer1;
u8 fitroutine = 0;
u8 fphysic = 0;
u8 fphytolog = 0;
u8 fremouse = 0;
u8 fremap = 0;
u8 fremouse2 = 0;
u8 vtiming = 0;
s16 savmouse[2];
u32 savmouse2[8];
u8 switchgo;
s16 backlarg;
u8 *wlogic;
s16 wlogx1;
s16 wlogx2;
s16 wlogy1;
s16 wlogy2;
s16 wloglarg;
s32 logx1 = 0;          // originx 0x0
s32 logx2 = 0x13f; // 0x013f00c7; // dimensions 319x199
s16 logy1 = 0;
s16 logy2 = 0xc7;
s16 loglarg = 0xa0; // 0x50 for st
u8 insid = 0;
u8 spag = 0;
u8 wpag = 0;
u8 *backmap;
s32 backx1;
s32 backx2;
s16 backy1;
s16 backy2;
u32 timeclock = 0;


void tdarkpal(u8 *paldata)
{
    // TODO: ...
//    u32 color;
//    u32 *in_A1;
//    s8 *pbVar4;
//    s16 *puVar6;
//    s16 uVar2;
//
//    s16 *puVar5 = dkpalet + (((s16)in_A1 - (s16)atpalet) >> 2) * 3;
//
//    do
//    {
//        color = *paldata;
//        paldata += 3;
//        bufrvb = color & 0xffffff00;
//        if ((bufrvb == 0) && (((0x100 < *puVar5 || (0x100 < puVar5[1])) || (0x100 < puVar5[2]))))
//        {
//            bufrvb = 0x1010001;
//        }
//
//        color = (bufrvb >> 0x18) * (u32)*puVar5;
//        if (0xffff < color)
//        {
//            color = 0xffff;
//        }
//
//        in_A1[0] = (s8)(color >> 9) & 0x7e;
//        puVar6 = puVar5 + 2;
//        color = (bufrvb >> 0x10 & 0xff) * (u32)puVar5[1];
//        if (0xffff < color) {
//            color = 0xffff;
//        }
//        in_A1[1] = (s8)(color >> 9) & 0x7e;
//        pbVar4 = in_A1 + 3;
//        in_A1[2] = 0;
//        puVar5 = puVar5 + 3;
//        color = (bufrvb >> 8 & 0xff) * (u32)*puVar6;
//        if (0xffff < color) {
//            color = 0xffff;
//        }
//
//        in_A1 += 4;
//        *pbVar4 = (s8)(color >> 9) & 0x7e;
//        uVar2 --;
//    }
//    while (uVar2 != 0xffff);
}

void topalet(void)
{
    ftopal = 0;

    if (palc == 1)
    {
        memcpy(mpalet, tpalet, 1024);
    }
    else
    {
        for (s32 i = 0; i < 1024; i++)
        {
            if (mpalet[i] == tpalet[i])
            {
                continue;
            }
            
            if (mpalet[i] < tpalet[i])
            {
                mpalet[i] += dpalet[i];
                if (mpalet[i] + dpalet[i] > tpalet[i])
                    mpalet[i] = tpalet[i];
            }
            else if (mpalet[i] > tpalet[i])
            {
                mpalet[i] -= dpalet[i];
                if (mpalet[i] - dpalet[i] < tpalet[i])
                    mpalet[i] = tpalet[i];
            }
        }
    }
    
    ftopal = 1;
}

void topalette(u8 *paldata, s32 duration)
{
    s16 colors = paldata[1];
    if (colors == 0) // 4 bit palette
    {
        if (thepalet != 0)
        {
            thepalet = 0;
            defpalet = 0;
            return;
        }
        
        palc = 0;
        u8 *palptr = &paldata[2];

        s16 to = 0;
        if (alis.platform.kind == EPlatformAtari)
        {
            for (s32 i = 0; i < 16; i++)
            {
                tpalet[to++] = (palptr[i * 2 + 0] & 0b00000111) << 5;
                tpalet[to++] = (palptr[i * 2 + 1] >> 4) << 5;
                tpalet[to++] = (palptr[i * 2 + 1] & 0b00000111) << 5;
            }
        }
        else
        {
            for (s32 i = 0; i < 16; i++)
            {
                tpalet[to++] = (palptr[i * 2 + 0] & 0b00001111) << 4;
                tpalet[to++] = (palptr[i * 2 + 1] >> 4) << 4;
                tpalet[to++] = (palptr[i * 2 + 1] & 0b00001111) << 4;
            }
        }
    }
    else // 8 bit palette
    {
        palc = 0;
        u8 offset = paldata[2];
        if (fdarkpal == 0)
        {
            memcpy(tpalet + (offset * 3), paldata + 4, colors * 3);
        }
        else
        {
            tdarkpal(paldata);
        }
    }
    
    thepalet = 0;
    defpalet = 0;
    
    if (duration != 0)
    {
        for (s32 i = 0; i < 256 * 3; i++)
        {
            dpalet[i] = abs(tpalet[i] - mpalet[i]) / duration;
        }

        topalet();
        palt = 1;
        palt0 = 1;
        palc = duration;
    }
    else
    {
        memcpy(mpalet, tpalet, 256 * 3);
        ftopal = 0xff;
    }
}

void toblackpal(s16 duration)
{
    if (nmode == 1 || nmode == 5 || nmode == 3 || nmode == 10)
    {
        return;
    }
    
    memset(tpalet, 0, 256 * 4);
    
    thepalet = 0;
    defpalet = 0;
    if (duration == 0)
    {
        memset(mpalet, 0, 256 * 4);
        ftopal = 0xff;
        palc = 0;
    }
    else
    {
        for (s32 i = 0; i < 256 * 3; i++)
        {
            dpalet[i] = abs(tpalet[i] - mpalet[i]) / (float)duration;
        }

        topalet();
        palt = 1;
        palt0 = 1;
        palc = duration;
    }
}

void savepal(s16 mode)
{
    if (mode < 0 && -3 < mode)
    {
        u8 *tgt = (mode != -1) ? svpalet2 : svpalet;
        memcpy(tgt, tpalet, 1024);
    }
}

void restorepal(s16 mode, s32 duration)
{
    u8 *src = (mode != -1) ? svpalet2 : svpalet;
    memcpy(tpalet, src, 1024);

    thepalet = 0;
    defpalet = 0;
    
    if (duration == 0)
    {
        memcpy(mpalet, src, 1024);
        ftopal = 0xff;
        palc = 0;
    }
    else
    {
        for (s32 i = 0; i < 256 * 3; i++)
        {
            dpalet[i] = abs(tpalet[i] - mpalet[i]) / (float)duration;
        }

        topalet();
        palt = 1;
        palt0 = 1;
        palc = duration;
    }
}

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

    s16 curidx;
    s16 scsprite = screen.ptscreen;
    
    while (scsprite != 0)
    {
        // if ((get_scr_state(scsprite) & 0x40) == 0)
        {
            debug(EDebugInfo, "  %s screen [0x%.4x]\n", (get_scr_state(scsprite) & 0x40) == 0 ? "visible" : "hidden", ELEMIDX(scsprite));

            u8 *bitmap = 0;
            sSprite *sprite;
            s16 lastidx2 = 0;
            s16 lastidx = 0;
            for (curidx = get_scr_screen_id(scsprite); curidx != 0; curidx = SPRITE_VAR(curidx)->link)
            {
                lastidx2 = lastidx;
                lastidx = curidx;
                sprite = SPRITE_VAR(curidx);

                sAlisScript *script = ENTSCR(sprite->script_ent);
                
                s32 addr = 0;
                s32 index = -1;
                
                bool deleted = false;
                
                if (script->vram_org)
                {
                    u8 *ptr = alis.mem + get_0x14_script_org_offset(script->vram_org);
                    s32 l = read32(ptr + 0xe);
                    s32 e = read16(ptr + l + 4);
                    
                    sAlisScript *prev = alis.script;
                    alis.script = script;
                    
                    for (s32 i = 0; i < e; i++)
                    {
                        addr = adresdes(i);
                        if (sprite->data == addr + script->data_org)
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
                u32 spnewad = sprite->newad & 0xffffff;
                if (spnewad)
                {
                    bitmap = (alis.mem + spnewad);
                    if (bitmap)
                    {
                        if (bitmap[0] == 1)
                            type = 1;

                        width = read16(bitmap + 2);
                        height = read16(bitmap + 4);
                    }
                }

                debug(EDebugInfo, "  %s%.2x %.4x type: %c idx: %.3d [x:%d y:%d d:%d w:%d h:%d] %s\n", deleted ? "!!" : "  ", (u8)sprite->numelem, ELEMIDX(curidx), type ? 'R' : 'B', index, sprite->newx, sprite->newy, sprite->newd, width, height, script->name);
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

    s16 previdx = 0;
    s16 curidx = get_0x18_unknown(alis.script->vram_org);
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
    image.finsprit = 0x7fff;

    s32 cursprit = image.debsprit;
    image.tvsprite = 0x8000;
    image.basesprite = image.debsprit + 0x8000;
    *(s32 *)(image.spritemem + cursprit + 0x0c) = 0;
    *(s16 *)(image.spritemem + cursprit + 0x16) = 0x13f;
    *(s16 *)(image.spritemem + cursprit + 0x18) = 199;
    *(s8 *) (image.spritemem + cursprit + 0x29) = 0;
    image.backsprite = 0x000e; // 0x8028;
    image.texsprite = 0x0042; // 0x8050;
    *(s32 *)(image.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (image.spritemem + cursprit + 0x50) = 0xfe;
    alis.mousflag = 0;
    image.libsprit = 0x78; // 0x8078;
    cursprit += 0x78;
    
    do
    {
        sSprite *sprite = SPRITE_VAR(cursprit);
        sprite->to_next = cursprit + 0x30;
        cursprit += 0x30;
    }
    while (cursprit < image.finsprit);
    
    *(s16 *)(SPRITEMEM_PTR + cursprit + -0x24) = 0;
}

u8 searchelem(s16 *curidx, s16 *previdx)
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

u8 testnum(s16 *curidx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

u8 nextnum(s16 *curidx, s16 *previdx)
{
    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

void createlem(s16 *curidx, s16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(image.libsprit);

    if (image.libsprit != 0)
    {
        s16 sprit = image.libsprit;
        s16 nextsprit = cursprvar->to_next;

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

void delprec(s16 elemidx)
{
    sSprite *cursprvar = SPRITE_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;
    if (elemidx == get_scr_screen_id(scridx))
    {
        set_scr_screen_id(scridx, cursprvar->screen_id);
        return;
    }

    s16 spridx = get_scr_screen_id(scridx);
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

void killelem(s16 *curidx, s16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
//    sAlisScript *s = ENTSCR(cursprvar->script_ent);
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

void getelem(s16 *newidx, s16 *oldidx)
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
    s32 addr = adresdes(idx);
    u8 *resourcedata = alis.mem + (alis.flagmain != 0 ? alis.main->data_org : alis.script->data_org) + addr;
//    s16 width = read16(resourcedata + 2);
//    s16 height = read16(resourcedata + 4);
//    printf(" putin: %d x %d ", width, height);

    s16 x = image.depx;
    s16 z = image.depz;
    s16 y = image.depy;
    
    if (resourcedata[0] > 0x80)
    {
        if (resourcedata[0] == 0xfe)
        {
            // TODO: handle palette
            debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
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
                *((u8 *)(&image.invert_x)) ^= 1;
            
            alis.fmuldes = 1;

            putin((u8)curelem);

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
        s16 newidx = 0;
        s16 oldidx = 0;
        
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
        cursprvar->data       = (alis.flagmain != 0 ? alis.main->data_org : alis.script->data_org) + addr;
        cursprvar->flaginvx   = (u8)image.invert_x;
        cursprvar->depx       = image.oldcx + image.depx;
        cursprvar->depy       = image.oldcy + image.depy;
        cursprvar->depz       = image.oldcz + image.depz;
        cursprvar->credon_off = get_0x25_credon_credoff(alis.script->vram_org);
        cursprvar->creducing  = get_0x27_creducing(alis.script->vram_org);
        cursprvar->clinking   = get_0x2a_clinking(alis.script->vram_org);
        cursprvar->cordspr    = get_0x2b_cordspr(alis.script->vram_org);
        cursprvar->chsprite   = get_0x2f_chsprite(alis.script->vram_org);
        cursprvar->script_ent = get_0x0e_script_ent(alis.script->vram_org);
        
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
        s16 newidx = 0;
        s16 oldidx = 0;

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
    for (u8 *strptr = alis.sd7; *strptr; strptr++)
    {
        put_char(*strptr);
    }
}

u32 tprintd0[] = { 0x3b9aca00, 0x05f5e100, 0x00989680, 0x000f4240, 0x000186a0, 0x00002710, 0x000003e8, 0x00000064, 0x0000000a, 0x00000001 };

void valtostr(u8 *string, s16 value)
{
    u8 *strptr = string;
    u8 *tmpptr;

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


s16 inilink(s16 elemidx)
{
    blocx1 = 0x7fff;
    blocy1 = 0x7fff;
    blocx2 = 0x8000;
    blocy2 = 0x8000;

    return SPRITE_VAR(elemidx)->clinking;
}

void calcfen(s16 elemidx1, s16 elemidx3)
{
    if (joints == 0)
    {
        sSprite *idx1sprvar = SPRITE_VAR(elemidx1);
        if (idx1sprvar->newd < 0)
            return;

        blocx1 = idx1sprvar->newx;
        blocy1 = idx1sprvar->newy;
        blocx2 = blocx1 + idx1sprvar->width;
        blocy2 = blocy1 + idx1sprvar->height;
    }
    
    sSprite *idx3sprvar = SPRITE_VAR(elemidx3);
    s16 tmpx = idx3sprvar->depx;
    s16 tmpy = idx3sprvar->depy;
    
    if (blocx1 <= tmpx && blocy1 <= tmpy && idx3sprvar->newx <= blocx2 && idx3sprvar->newy <= blocy2)
    {
        fenx1 = blocx1;
        if (blocx1 < idx3sprvar->newx)
            fenx1 = idx3sprvar->newx;
        
        feny1 = blocy1;
        if ((s16)blocy1 < idx3sprvar->newy)
            feny1 = idx3sprvar->newy;
        
        fenx2 = blocx2;
        if (tmpx < blocx2)
            fenx2 = tmpx;
        
        feny2 = blocy2;
        if (tmpy < blocy2)
            feny2 = tmpy;
    }
}

u8 clipfen(sSprite *sprite)
{
    fclip = 0;
    s16 spritex1 = sprite->newx;
    if (fenx2 < spritex1)
        return fclip;

    s16 spritey1 = sprite->newy;
    if (feny2 < spritey1)
        return fclip;

    s16 spritex2 = sprite->depx;
    if (spritex2 < fenx1)
        return fclip;

    s16 spritey2 = sprite->depy;
    if (spritey2 < feny1)
        return fclip;

    if (spritex1 < fenx1)
        spritex1 = fenx1;

    if (spritey1 < feny1)
        spritey1 = feny1;

    if (fenx2 < spritex2)
        spritex2 = fenx2;

    if (feny2 < spritey2)
        spritey2 = feny2;

    clipx1 = spritex1;
    clipy1 = spritey1;
    clipx2 = spritex2;
    clipy2 = spritey2;
    
    clipl = (spritex1 - spritex2) + 1;
    cliph = (spritey1 - spritey2) + 1;
    fclip = 1;
    return fclip;
}

s16 rangesprite(s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    sSprite *idx1sprvar = SPRITE_VAR(elemidx1);
    s16 newd1  =  idx1sprvar->newd;
    s8 cordspr1 = idx1sprvar->cordspr;
    s8 numelem1 = idx1sprvar->numelem;
    if (wback != 0 && elemidx1 != image.backsprite && backprof <= newd1)
    {
        pback = 1;
    }

    s16 previdx3 = elemidx3;
    s16 nextidx3 = SPRITE_VAR(elemidx3)->link;
    sSprite *idx3sprvar = NULL;

    while (nextidx3 != 0)
    {
        idx3sprvar = SPRITE_VAR(nextidx3);
        s16 newd2 =   idx3sprvar->newd;
        s8 cordspr2 = idx3sprvar->cordspr;
        s8 numelem2 = idx3sprvar->numelem;

        if (-1 < idx3sprvar->state && newd2 <= newd1 && (newd2 < newd1 || (cordspr2 <= cordspr1 && (cordspr2 < cordspr1 || (numelem2 <= numelem1 && (numelem2 < numelem1 || -1 == idx1sprvar->state))))))
        {
            break;
        }
        
        previdx3 = nextidx3;
        nextidx3 = SPRITE_VAR(nextidx3)->link;
    }

    idx1sprvar->state = 0;
    idx1sprvar->link = nextidx3;
    SPRITE_VAR(previdx3)->link = elemidx1;

    if (elemidx2 == previdx3)
    {
        elemidx2 = elemidx1;
    }
    
    return elemidx2;
}

void tstjoints(s16 elemidx)
{
    sSprite *sprvar = SPRITE_VAR(elemidx);
    s16 tmpx = sprvar->newx + sprvar->width;
    s16 tmpy = sprvar->newy + sprvar->height;
    
    if (-1 < sprvar->newd && -1 < newd && sprvar->newx <= newx + newl && sprvar->newy <= newy + newh && newx <= tmpx && newy <= tmpy)
    {
        blocx1 = sprvar->newx;
        if (newx < sprvar->newx)
            blocx1 = newx;

        blocy1 = sprvar->newy;
        if (newy < sprvar->newy)
            blocy1 = newy;

        if (tmpx < (s16)(newx + newl))
            tmpx = newx + newl;

        if (tmpy < (s16)(newy + newh))
            tmpy = newy + newh;

        blocx2 = tmpx;
        blocy2 = tmpy;
        joints = 1;

        return;
    }
    
    joints = 0;
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

void depscreen(s16 scene, s16 elemidx)
{
    set_scr_depx(scene, get_scr_depx(scene) + get_scr_unknown0x2a(scene));
    set_scr_depy(scene, get_scr_depy(scene) + get_scr_unknown0x2c(scene));
    set_scr_depz(scene, get_scr_depz(scene) + get_scr_unknown0x2e(scene));
    
    sSprite *elemsprvar = NULL;
    
    while (elemidx != 0)
    {
        elemsprvar = SPRITE_VAR(elemidx);
        
        if (-1 < (s8)elemsprvar->chsprite && elemsprvar->state == 0)
        {
            elemsprvar->state = 2;
        }

        elemidx = elemsprvar->link;
    }
}

void deptopix(s16 scene, s16 elemidx)
{
    if ((get_scr_numelem(scene) & 2) == 0)
    {
        sSprite *elemsprvar = SPRITE_VAR(elemidx);
        
        scene = elemsprvar->screen_id;
        newf = elemsprvar->flaginvx;
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
            s16 scridx = (tmpdepz >> (elemsprvar->credon_off & 0x3f)) + (s16)elemsprvar->creducing;
            if (scridx < 0)
            {
                offset = 0;
            }
            else
            {
                if (get_scr_clinking(scene) < scridx)
                {
                    scridx = get_scr_clinking(scene);
                }
                
                offset = scridx << 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0a(scene);
        tmpdepy = (tmpdepy >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0c(scene);

        newad = (elemsprvar->data & 0xffffff) + offset;
        u8 *spritedata = (alis.mem + newad);
        if (spritedata[0] == '\x03')
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4);
            tmpdepy += read16(spritedata + 6);
            if (spritedata[1] != 0)
            {
                newf = newf ^ 1;
            }
            
            newad = (newad + (read16(spritedata + 2) << 2));
            spritedata = (alis.mem + newad);
        }
        
        newl = read16(spritedata + 2);
        newh = read16(spritedata + 4);
        newzoomx = 0;
        newzoomy = 0;
        newx = tmpdepx - (newl >> 1);
        newy = tmpdepy - (newh >> 1);
        newd = tmpdepz;
    }
}

void waitphysic(void)
{
    do {} while (fphysic != 0);
}

void trsfen(u8 *src, u8 *tgt)
{
    src += fenx1 + feny1 * 320;
    tgt += fenx1 + feny1 * 320;
    
    s16 skip = 320 - (fenx2 - fenx1);

    for (s32 y = feny1; y < feny2; y++)
    {
        for (s32 x = fenx1; x < fenx2; x++, src++, tgt++)
        {
            *tgt = *src;
        }
        
        tgt += skip;
        src += skip;
    }
}

void phytolog(void)
{
    fenx1 = 0;
    feny1 = 0;
    fenx2 = 0x13f;
    feny2 = 199;
    trsfen(physic, logic);
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
    // fenx2 = fenlargw * 4 + fenx1 - 1;
    trsfen(physic, logic);
}

void memfen(void)
{
    ptabfen[0] = fenx1;
    ptabfen[1] = fenx2;
    ptabfen[2] = feny1;
    ptabfen[3] = feny2;
    ptabfen += 4;

    s16 *endtabfen = tabfen + sizeof(tabfen);
    if (ptabfen < endtabfen)
    {
        return;
    }

    // error
    debug(EDebugError, "ptabfen pointing outside of available mem!\n");
}

void oldfen(void)
{
    for (s16 *tabptr = tabfen; tabptr < ptabfen; tabptr += 4)
    {
        fenx1 = tabptr[0];
        fenx2 = tabptr[1];
        feny1 = tabptr[2];
        feny2 = tabptr[3];
        tvtofen();
    }
}

void setphysic(void)
{
    if (insid == 0)
    {
        host.pixelbuf.data = physic;
    }

    bufpack = logic;
    fphysic = 1;
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

void addlink(s16 elemidx)
{
    sSprite *elemsprvar = SPRITE_VAR(elemidx);
    if (-1 < elemsprvar->newd)
    {
        s16 tmp = elemsprvar->newx;
        if (tmp <= blocx1)
            blocx1 = tmp;

        tmp = tmp + elemsprvar->width;
        if (blocx2 <= tmp)
            blocx2 = tmp;

        tmp = elemsprvar->newy;
        if (tmp <= blocy1)
            blocy1 = tmp;

        tmp = tmp + elemsprvar->height;
        if (blocy2 <= tmp)
            blocy2 = tmp;
    }
}

s16 inouvlink(s16 scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    deptopix(scene, elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newy = newy;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

s16 iremplink(s16 scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    addlink(elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    if (wback != 0 && backprof <= elem1sprvar->newd)
    {
        pback = 1;
        return inouvlink(scene, elemidx1, elemidx2, elemidx3);
    }
    
    deptopix(scene, elemidx1);
    
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newy = newy;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

s16 iefflink(s16 elemidx1, s16 elemidx2)
{
    addlink(elemidx1);

    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;
    elem1sprvar->to_next = image.libsprit;
    
    image.libsprit = elemidx1;
    if (wback != 0 && backprof <= elem1sprvar->newd)
    {
        pback = 1;
    }
    
    return elemidx2;
}

s16 scdirect = 0;
u8 fdoland = 0;

void clrfen(void)
{
    s16 tmpx = fenx2 - fenx1;
    // s16 tmpy = feny2 - feny1;
    
    for (s16 y = feny1; y < feny2; y++)
    {
        memset(physic + fenx1 + y * 320, 0, tmpx);
    }
}

void clipback(void)
{
    if (clipy1 < ((s16 *)(&backx1))[1])
    {
        if (clipy2 < ((s16 *)(&backx1))[1])
        {
            return;
        }
    }
    else
    {
        if (clipy2 <= ((s16 *)(&backx2))[1])
        {
            cback = 1;
            return;
        }
        
        if (((s16 *)(&backx2))[1] < clipy1)
        {
            return;
        }
    }
    
    cback = -1;
    return;
}

void destofen(sSprite *sprite)
{
    u32 spnewad = sprite->newad & 0xffffff;
    if (spnewad == 0)
        return;
    
    u8 *bitmap = (alis.mem + spnewad);
    if (*bitmap < 0)
        return;

    s16 posx1 = sprite->newx;
    s16 posy1 = sprite->newy;
    s16 xpos2 = posx1 + (s16)read16(bitmap + 2);
    s16 ypos2 = posy1 + (s16)read16(bitmap + 4);

    s8 flip = sprite->newf;
    s32 width = sprite->width + 1;
    s32 height = sprite->height + 1;

//     s16 spridx = (u8 *)sprite - SPRITEMEM_PTR;
//    sAlisScript *s = ENTSCR(sprite->script_ent);
//    alis.script = s;
//
//    s16 index = -1;
//
//    sAlisScript *prevscript = alis.script;
//
//    s32 addr = 0;
//
//    u8 *ptr = alis.mem + get_0x14_script_org_offset(alis.script->vram_org);
//    s32 l = read32(ptr + 0xe);
//    s32 e = read16(ptr + l + 4);
//
//    for (s32 i = 0; i < e; i++)
//    {
//        addr = adresdes(i);
//        if (sprite->data == addr + s->data_org)
//        {
//            index = i;
//            break;
//        }
//    }
//
//    if (index < 0)
//    {
//        DRAW_TRACE("UNKNOWN ");
//    }
//    else
//    {
//        DRAW_TRACE2("\n");
//        DRAW_TRACE("%s RSRC %d ", s->name, index);
//    }
//
//    alis.script = prevscript;
//
////    printf("\nN %.4x [%.2x %.2x] %d x %d %d x %d", ELEMIDX(spridx), bitmap[0], bitmap[1], posx1, posy1, width, height);
//    printf("\nRaw %.2x %.4x type: %c idx: %.3d [x:%d y:%d d:%d w:%d h:%d] %s", (u8)sprite->numelem, ELEMIDX(spridx), bitmap[0] == 1 ? 'R' : 'B', index, posx1, posy1, sprite->newd, width, height, s->name);

    if (clipx2 < posx1)
        return;

    if (clipy2 < posy1)
        return;

    if (xpos2 < clipx1)
        return;

    if (ypos2 < clipx1)
        return;

    blocx1 = posx1;
    if (posx1 < clipx1)
        blocx1 = clipx1;

    blocy1 = posy1;
    if (posy1 < clipy1)
        blocy1 = clipy1;

    blocx2 = xpos2;
    if (clipx2 < xpos2)
        blocx2 = clipx2;

    blocy2 = ypos2;
    if (clipy2 < ypos2)
        blocy2 = clipy2;

    u8 *at = bitmap + 6;

    u8 color;
    u8 clear = 0;
    u8 palidx = 0;

    s32 bmpy1 = 0;
    s32 bmpy2 = height;
    if (posy1 < 0)
    {
        bmpy2 += posy1;
        bmpy1 -= posy1;
    }
    
    s32 bmpx1 = 0;
    s32 bmpx2 = width;
    if (blocx1 < 0)
    {
        bmpx1 -= blocx1;
        bmpx2 += blocx1;
    }
    
    if (blocx1 > posx1)
    {
        bmpx1 += (blocx1 - posx1);
        bmpx2 -= (blocx1 - posx1);
    }
    
    if (blocx2 < xpos2)
    {
        bmpx2 -= (xpos2 - blocx2);
    }

    if (blocy2 < ypos2)
    {
        bmpy2 -= (ypos2 - blocy2);
    }
    
//    printf("\nMod %.2x %.4x type: %c idx: %.3d [x:%d y:%d d:%d w:%d h:%d] %s", (u8)sprite->numelem, ELEMIDX(spridx), bitmap[0] == 1 ? 'R' : 'B', index, posx1, posy1, sprite->newd, width, height, s->name);
    
    // NOTE: just a hack to write directly to output buffer
    
    switch (bitmap[0])
    {
        case 0x01:
        {
            // rectangle
            
            color = bitmap[1];
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
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
            
            at = bitmap + 6;
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    if (color != clear)
                    {
                        tgt = logic + (w + posx1) + ((posy1 + h) * host.pixelbuf.w);
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
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    if (color != clear)
                        *tgt = palidx + color;
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
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    color = *(at + (flip ? width - (w + 1) : w) + h * width);
                    if (color != clear)
                        *tgt = color;
                }
            }
            
            break;
        }
            
        case 0x40:
        {
            // FLI video
            
            uint32_t size1 = read32(bitmap + 2);
            s8 *fliname = (s8 *)&bitmap[6];
            debug(EDebugVerbose, "FLI video (%s) %d bytes [", fliname, size1);

            uint32_t size2 = (*(uint32_t *)(&bitmap[32]));
            uint16_t frames = (*(uint16_t *)(&bitmap[38]));

            debug(EDebugVerbose, "size: %d frames: %d]\n", size2, frames);

            // TODO: ...
            
            break;
        }
    }
}

void calctop(void)
{
    return;
}

void fentotv(void)
{
    trsfen(logic, physic);
}

void fenetre(s16 scene, s16 elemidx1, s16 elemidx3)
{
    s16 tmpidx;
    s16 scridx;
    sSprite *sprite;

    if ((get_scr_state(scene) & 0x40) != 0)
    {
        return;
    }

    scdirect = 0;
    calcfen(elemidx1, elemidx3);
    
    {
        if ((get_scr_numelem(scene) & 2) != 0)
        {
            fdoland = 1;
        }
        
        scridx = screen.ptscreen;
        tmpidx = screen.ptscreen;
        while (tmpidx != 0)
        {
            if ((get_scr_state(scridx) & 0x40U) == 0)
            {
                tmpidx = get_scr_screen_id(scridx);
                elemidx1 = tmpidx;
                if (tmpidx != 0)
                {
                    clipfen(SPRITE_VAR(tmpidx));
//                    if (fclip != 0)
                    {
                        if ((get_scr_numelem(scridx) & 2U) == 0)
                        {
                            cback = 0;
                            if ((get_scr_numelem(scridx) & 4U) != 0)
                            {
                                clipback();
                            }

                            if ((get_scr_numelem(scridx) & 0x40U) == 0)
                            {
                                clrfen();
                            }

                            if (cback)
                            {
                                if (cback < 0)
                                {
                                    if ((wback != 0) && (pback != 0))
                                    {
                                        wlogic = backmap;
                                        wlogx1 = backx1;
                                        wlogx2 = backx2;
                                        wlogy1 = backy1;
                                        wlogy2 = backy2;
                                        wloglarg = backlarg;
                                        
                                        if (clipy1 <= backx1)
                                            clipy1 = backx1;

                                        if (backx2 <= clipy2)
                                            clipy2 = backx2;

                                        sprite = SPRITE_VAR(tmpidx);
                                        while (true)
                                        {
                                            tmpidx = sprite->link;
                                            if ((tmpidx == 0) || (tmpidx == image.backsprite))
                                                break;

                                            sprite = SPRITE_VAR(tmpidx);
                                            if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                            {
//                                                DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(tmpidx));
                                                destofen(sprite);
                                            }
                                        }

                                        wlogic = logic;
                                        wlogx1 = logx1;
                                        wlogx2 = logx2;
                                        wlogy1 = logy1;
                                        wlogy2 = logy2;
                                        wloglarg = loglarg;
                                    }
                                }
                                else
                                {
                                    if (pback == 0)
                                        goto fenetre31;

                                    wlogic = backmap;
                                    wlogx1 = backx1;
                                    wlogx2 = backx2;
                                    wlogy1 = backy1;
                                    wlogy2 = backy2;
                                    wloglarg = backlarg;
                                }
                            }

                            sprite = SPRITE_VAR(tmpidx);
                            while (true)
                            {
                                tmpidx = sprite->link;
                                if (tmpidx == 0)
                                    break;

                                if ((sback != 0) && (tmpidx == image.backsprite))
                                {
                                    wlogic = logic;
                                    wlogx1 = logx1;
                                    wlogx2 = logx2;
                                    wlogy1 = logy1;
                                    wlogy2 = logy2;
                                    wloglarg = loglarg;
                                }
  fenetre31:
                                sprite = SPRITE_VAR(tmpidx);
                                if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                {
//                                    DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(tmpidx));
                                    destofen(sprite);
                                    switchgo = 1;
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

            tmpidx = get_scr_to_next(scridx);
            scridx = tmpidx;
        }

        // TODO: mouse
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
                if (wpag == 0)
                {
                    memfen();
                }

                return;
            }

            if (wpag == 0)
            {
                fentotv();
            }
        }

        // TODO: mouse
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

void affiscr(s16 scene, s16 elemidx3)
{
    s8 state;
    s16 elemidx1;
    s16 elemidx2;
    s16 tmpidx1;
    s16 tmpidx2;
    s16 tmpidx3;
    u8 issprit;
    u8 isback;
    
//    if (a2[0x84] != 0)
//    {
//        a2 = folscreen(a2);
//    }
    
    if ((fremap != 0) || ((s8)get_scr_state(scene) < 0))
    {
        depscreen(scene, elemidx3);
    }
    
    if ((get_scr_numelem(scene) & 2) == 0 || (get_scr_state(scene) & 0x80U) == 0)
    {
        wback = (get_scr_numelem(scene) & 4) != 0;
        wpag = 0;
        if ((get_scr_state(scene) & 0x20U) != 0)
        {
            wpag = 1;
            spag --;
            if (spag == 0)
            {
                wpag = -1;
            }
        }
        
        elemidx1 = elemidx3;
        
        if ((get_scr_numelem(scene) & 0x10) == 0)
        {
affiscin:
            do
            {
                tmpidx2 = elemidx1;
                elemidx1 = SPRITE_VAR(tmpidx2)->link;
                if (elemidx1 == 0)
                    goto affiscr1;
                
                state = SPRITE_VAR(elemidx1)->state;
                if (state != 0)
                {
                    joints = 0;
                    pback = 0;
                    if (state == 2)
                    {
                        tmpidx1 = elemidx1;
                        tmpidx3 = inilink(elemidx1);
//                        printf("INILINK A: %.x == %.x", tmpidx3, elemidx1);
//                        printf(" [if (state == 2) %.6x] ", tmpidx3);
                        if (tmpidx3 < 0)
                        {
                            isback = wback == 0;
                            if (!isback)
                            {
                                isback = backprof == SPRITE_VAR(elemidx1)->newd;
                                if (backprof <= SPRITE_VAR(elemidx1)->newd)
                                {
                                    pback = 1;
                                    isback = false;
                                }
                            }
                            
                            deptopix(scene,elemidx1);
                            tstjoints(elemidx1);
                            if (joints == 0)
                            {
                                SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                                fenetre(scene,elemidx1,elemidx3);
                            }
                            else
                            {
                                SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                            }
                            
                            sSprite *sprvar = SPRITE_VAR(elemidx1);
                            sprvar->newad = newad;
                            sprvar->newx = newx;
                            sprvar->newy = newy;
                            sprvar->newd = newd;
                            sprvar->newf = newf;
                            sprvar->width = newl;
                            sprvar->height = newh;
                            sprvar->newzoomx = newzoomx;
                            sprvar->newzoomy = newzoomy;
                            
                            tmpidx2 = rangesprite(elemidx1, tmpidx2, elemidx3);
                            fenetre(scene, elemidx1, elemidx3);
                            elemidx1 = tmpidx2;
                            goto affiscin;
                        }
                        
                        tmpidx1 = iremplink(scene, tmpidx1, tmpidx2, elemidx3);
                    }
                    else
                    {
                        if (state == -1)
                        {
                            tmpidx1 = elemidx1;
                            tmpidx3 = inilink(elemidx1);
//                            printf("INILINK B: %.x == %.x", tmpidx3, elemidx1);
                            tmpidx1 = inouvlink(scene, tmpidx1, tmpidx2, elemidx3);
                        }
                        else
                        {
                            tmpidx1 = elemidx1;
                            tmpidx3 = inilink(elemidx1);
//                            printf("INILINK C: %.x == %.x", tmpidx3, elemidx1);
                            tmpidx1 = iefflink(tmpidx1, tmpidx2);
                        }
                        
                        if (tmpidx3 < 0)
                        {
                            joints = 1;
                            fenetre(scene, elemidx1, elemidx3);
                            elemidx1 = tmpidx2;
                            goto affiscin;
                        }
                    }
                    
                    elemidx2 = tmpidx1;
                    while ((tmpidx1 = SPRITE_VAR(elemidx2)->link) != 0)
                    {
//                        if (tmpidx3 == SPRITE_VAR(tmpidx1)->clinking)
//                        {
//                            printf("LINK: %.x == %.x", tmpidx3, SPRITE_VAR(tmpidx1)->clinking);
//                        }
//                        else
//                        {
//                            s16 elem = ELEMIDX(tmpidx1);
//                            SpriteVariables *stest = SPRITE_VAR(tmpidx1);
//
//                            printf("LINK: %.x != %.x", tmpidx3, SPRITE_VAR(tmpidx1)->clinking);
//                        }
                        
                        if (1) // draw everything
//                        if (tmpidx3 == SPRITE_VAR(tmpidx1)->clinking)
                        {
                            state = SPRITE_VAR(tmpidx1)->state;
                            if (state != 0)
                            {
                                if (state < 0)
                                {
                                    tmpidx1 = inouvlink(scene, tmpidx1, elemidx2, elemidx3);
                                }
                                else if (state == 2)
                                {
                                    tmpidx1 = iremplink(scene, tmpidx1, elemidx2, elemidx3);
                                }
                                else
                                {
                                    tmpidx1 = iefflink(tmpidx1, elemidx2);
                                }
                            }
                        }
                        
                        elemidx2 = tmpidx1;
                    }
                    
                    joints = 1;
                    fenetre(scene,elemidx1,elemidx3);
                    elemidx1 = tmpidx2;
                }
            }
            while(true);
        }
        
        fenx1 = get_scr_newx(scene);
        feny1 = get_scr_newy(scene);
        fenx2 = fenx1 + get_scr_width(scene);
        feny2 = feny1 + get_scr_height(scene);
        clipl = (fenx2 - fenx1) + 1;
        cliph = (feny2 - feny1) + 1;

//        fenlargw = clipl >> 2;

        clipx1 = fenx1;
        clipy1 = feny1;
        clipx2 = fenx2;
        clipy2 = feny2;
        
        if ((get_scr_numelem(scene) & 0x40) == 0)
        {
            clrfen();
        }
        
        tmpidx2 = elemidx1;
        while ((elemidx1 = SPRITE_VAR(tmpidx2)->link) != 0)
        {
            if (SPRITE_VAR(elemidx1)->state != 0)
            {
                if (SPRITE_VAR(elemidx1)->state == 1)
                {
                    SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                    SPRITE_VAR(elemidx1)->to_next = image.libsprit;
                    image.libsprit = elemidx1;
                    elemidx1 = tmpidx2;
                }
                else
                {
                    deptopix(scene,elemidx1);
                    SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;

                    sSprite *sprvar = SPRITE_VAR(elemidx1);
                    sprvar->newad = newad;
                    sprvar->newx = newx;
                    sprvar->newy = newy;
                    sprvar->newd = newd;
                    sprvar->newf = newf;
                    sprvar->width = newl;
                    sprvar->height = newh;
                    sprvar->newzoomx = newzoomx;
                    sprvar->newzoomy = newzoomy;
                    elemidx1 = rangesprite(elemidx1,tmpidx2,elemidx3);
                }
            }
        }
        
        issprit = screen.ptscreen == 0;
        elemidx1 = screen.ptscreen;
    }
    else
    {
        issprit = get_scr_to_next(scene) == 0;
        elemidx1 = get_scr_to_next(scene);
    }
    
    while (!issprit)
    {
        if (((get_scr_state(elemidx1) & 0x40U) == 0) && ((tmpidx2 = get_scr_screen_id(elemidx1)) != 0))
        {
            clipfen(SPRITE_VAR(tmpidx2));
            if (fclip != 0)
            {
                while ((tmpidx2 = SPRITE_VAR(tmpidx2)->link) != 0)
                {
                    if (-1 < SPRITE_VAR(tmpidx2)->state && -1 < SPRITE_VAR(tmpidx2)->newf && -1 < SPRITE_VAR(tmpidx2)->newd)
                    {
                        destofen(SPRITE_VAR(tmpidx2));
                        switchgo = 1;
                    }
                }
            }
        }
        
        issprit = get_scr_to_next(elemidx1) == 0;
        elemidx1 = get_scr_to_next(elemidx1);
    }
    
    if ((alis.fswitch == 0) && (wpag == 0))
    {
        fentotv();
    }
    
  affiscr1:
    
    if (wpag < 0)
    {
        set_scr_state(scene, get_scr_state(scene) & 0xdf);
        fenx1 = get_scr_newx(scene);
        feny1 = get_scr_newy(scene);
        fenx2 = get_scr_newx(scene) + get_scr_width(scene);
        feny2 = get_scr_newy(scene) + get_scr_height(scene);
        // scrolpage(a0,CONCAT22(uVar2,feny2));
    }
    
    set_scr_state(scene, get_scr_state(scene) & 0x7f);
}

void itroutine(void)
{
    u8 prevtiming = vtiming;
    timeclock ++;
    fitroutine = 1;
    vtiming = vtiming + 1;
    if (vtiming == 0) {
        vtiming = prevtiming;
    }
    
    if (palc != 0 && (--palt) == 0)
    {
        palt = palt0;
        topalet();
        palc --;
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
    
    fitroutine = 0;
}

void waitframe(void) {
    
    struct timeval now, start;
    gettimeofday(&start, NULL);
    
    while ((void)(gettimeofday(&now, NULL)), ((now.tv_sec * 1000000 + now.tv_usec) - (alis.time.tv_sec * 1000000 + alis.time.tv_usec) < FRAME_TICKS * alis.ctiming)) {
        usleep(1000);
    }

    alis.time = now;
}

void draw(void)
{
//    log_sprites();
    
    VERIFYINTEGRITY;
    
    waitframe();
    
//    waitphysic();
    if ((alis.fswitch != 0) && (fphytolog != 0))
    {
        fphytolog = 0;
        phytolog();
    }
    
    // s8 prevmouse = fremouse;
    
    // wait for vblank
    // do {} while (vtiming < timing);
    
    vtiming = 0;
    
    if (alis.fswitch != 0)
    {
        // TODO: mouse
        if (fmouse < 0)
        {
            oldfen();
        }
        else
        {
//            do { } while (fremouse2 != 0);
//
//            fremouse ++;
//            if (fremouse != 0)
//            {
//                fremouse = prevmouse;
//            }
//
            oldfen();
//            fremouse2 = 1;
//            if (fmouse != 2)
//            {
//                mouserase();
//            }
//
//            fremouse2 = 0;
//            fremouse = -1;
//            if (fmouse == 0)
//            {
//                fmouse = -1;
//            }
        }

        ptabfen = tabfen;
    }
    
    switchgo = 0;
    wlogic = logic;
    wlogx1 = logx1;
    wlogx2 = logx2;
    wlogy1 = logy1;
    wlogy2 = logy2;
    wloglarg = loglarg;
    
    // hack
    fremap = 1;
    
    s16 scnidx = screen.ptscreen;
//    prevmouse = fremouse;
    u8 *oldphys = physic;
    
    while (scnidx != 0)
    {
        if ((get_scr_state(scnidx) & 0x40) == 0)
        {
            s16 sprite3 = get_scr_screen_id(scnidx);
            affiscr(scnidx, sprite3);
        }
        
        scnidx = get_scr_to_next(scnidx);
    }

    fremap = 0;
    if (alis.fswitch != 0)
    {
//        do
//        {
//            if (switchgo == 0)
//            {
//                fremouse = prevmouse;
//                fremap = 0;
//            }
//
//            fremouse = prevmouse;
//        }
//        while (((fmouse != 0) && (-1 < fmouse)) && ((void)(fremouse = prevmouse + 1), fremouse != 0));
//
        physic = logic;
        logic = oldphys;
        setphysic();
        
//        if ((fmouse != 0) && (-1 < fmouse))
//        {
//            fremouse2 = 1;
//            fremouse = -1;
//        }
    }
    
    sys_render(host.pixelbuf);

    VERIFYINTEGRITY;
}

u8 *buffer = 0;

void setmpalet(void)
{
    ftopal = 0xff;
    thepalet = 0;
    defpalet = 0;
}

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
