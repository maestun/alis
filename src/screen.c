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
#include "mem.h"
#include "screen.h"

sScreen screen = { .ptscreen = 0 };

typedef enum {
    EScreenState        = 0x00,
    EScreenNumElem      = 0x01,
    EScreenID           = 0x02,
    EScreenToNext       = 0x04,
    EScreenLink         = 0x06,
    EScreenUnknown0x08  = 0x08,
    EScreenUnknown0x0a  = 0x0a,
    EScreenUnknown0x0c  = 0x0c,
    EScreenNewX         = 0x0e,
    EScreenNewY         = 0x10,
    EScreenWidth        = 0x12,
    EScreenHeight       = 0x14,
    EScreenDepX         = 0x16,
    EScreenDepY         = 0x18,
    EScreenDepZ         = 0x1a,
    EScreenCRedOff      = 0x1c,
    EScreenCReducing    = 0x1d,
    EScreenCLinking     = 0x1e,
    EScreenUnknown0x20  = 0x20,
    EScreenUnknown0x21  = 0x21,
    EScreenUnknown0x22  = 0x22,
    EScreenUnknown0x23  = 0x23,
    EScreenUnknown0x24  = 0x24,
    EScreenUnknown0x25  = 0x25,
    EScreenUnknown0x26  = 0x26,
    EScreenUnknown0x27  = 0x27,
    EScreenUnknown0x28  = 0x28,
    EScreenUnknown0x29  = 0x29,
    EScreenUnknown0x2a  = 0x2a,
    EScreenUnknown0x2c  = 0x2c,
    EScreenUnknown0x2e  = 0x2e,
} eScreenVars;


s16     get_screen_var(eScreenVars var, u32 screen_idx)              { return xread16(alis.basemain + screen_idx + var); }
void    set_screen_var(eScreenVars var, u32 screen_idx, s16 val)     { xwrite16(alis.basemain + screen_idx + var, val); }

void scadd(s16 scridx)
{
    set_scr_to_next(scridx, 0);
    
    u16 nextscreen = screen.ptscreen;
    if (screen.ptscreen == 0)
    {
        screen.ptscreen = scridx;
        return;
    }
    
    s16 curscreen;

    do
    {
        curscreen = nextscreen;
        nextscreen = get_scr_to_next(curscreen);
    }
    while (nextscreen != 0);
    
    set_scr_to_next(curscreen, scridx);
}

void scbreak(s16 scridx)
{
    if (screen.ptscreen == 0)
        return;
    
    s16 prevptscreen = screen.ptscreen;
    if (scridx == screen.ptscreen)
    {
        screen.ptscreen = get_scr_to_next(scridx);
        return;
    }
    
    s16 nextptscreen;

    do
    {
        nextptscreen = get_scr_to_next(prevptscreen);
        if (nextptscreen == scridx)
        {
            set_scr_to_next(prevptscreen, get_scr_to_next(scridx));
            return;
        }
        
        prevptscreen = nextptscreen;
    }
    while (nextptscreen != 0);
}

void scdosprite(s16 scridx)
{
    s16 spritidx = get_scr_screen_id(scridx);

    sSprite *sprite = SPRITE_VAR(spritidx);
    sprite->numelem = get_scr_numelem(scridx);
    
    s16 x = get_scr_newx(scridx);
    s16 y = get_scr_newy(scridx);
    s16 w = get_scr_width(scridx);
    s16 h = get_scr_height(scridx);

    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x, &y);
        mac_update_pos(&w, &h);
    }
    
    sprite->newx = x;
    sprite->newy = y;
    sprite->newd = 0x7fff;
    sprite->depx = x + w;
    sprite->depy = y + h;
}

void vectoriel(s16 scridx)
{
    set_scr_unknown0x26(scridx, get_scr_unknown0x21(scridx) * get_scr_unknown0x25(scridx) - get_scr_unknown0x24(scridx) * get_scr_unknown0x22(scridx));
    set_scr_unknown0x27(scridx, get_scr_unknown0x22(scridx) * get_scr_unknown0x23(scridx) - get_scr_unknown0x25(scridx) * get_scr_unknown0x20(scridx));
    set_scr_unknown0x28(scridx, get_scr_unknown0x20(scridx) * get_scr_unknown0x24(scridx) - get_scr_unknown0x23(scridx) * get_scr_unknown0x21(scridx));
}
