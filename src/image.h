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

#pragma once

#include "alis.h"
#include "config.h"

// to be consistent with old code

PACK_PUSH typedef struct PACK_ATTR {

    s8 state;               // 0x00
    s8 numelem;             // 0x01
    s16 screen_id;          // 0x02
    u16 to_next;            // 0x04
    u16 link;               // 0x06
    u32 newad : 24;         // 0x08
    s8 newf : 8;            // 0x08 + 3
    s16 newx;               // 0x0c
    s16 newy;               // 0x0e
    s16 newd;               // 0x10
    u32 data : 24;          // 0x12
    u8 flaginvx : 8;        // 0x12 + 3 // +0 on big endian machines
    s16 depx;               // 0x16
    s16 depy;               // 0x18
    s16 depz;               // 0x1a
    s8 credon_off;          // 0x1c
    u8 creducing;           // 0x1d
    s16 clinking;           // 0x1e
    u8 cordspr;             // 0x20
    u8 chsprite;            // 0x21
    u16 script_ent;         // 0x22
    u32 sprite_0x28;        // 0x24
    s16 width;              // 0x28
    s16 height;             // 0x2a
    u16 newzoomx;           // 0x2c
    u16 newzoomy;           // 0x2e
} sSprite; PACK_POP

typedef struct {
    s32 x;
    s32 y;
    s32 z;
} sVector;

typedef struct {
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
} sRect;

// sColorARGB:  SDL2 textures, big-endian SDL1 32-bit — val = 0xAARRGGBB
// sColorABGR:  little-endian SDL1 32-bit surfaces    — val = 0xAABBGGRR
// sColorRGBA32: SDL_Color compatible (SDL_PIXELFORMAT_RGBA32) — memory always [R,G,B,A]
// sColor565:   Falcon 16-bit                         — val = RRRRRGGGGGGBBBBB

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
typedef union { struct { u32 a:8; u32 r:8; u32 g:8; u32 b:8; }; u32 val; } sColorARGB;
typedef union { struct { u32 b:8; u32 g:8; u32 r:8; u32 a:8; }; u32 val; } sColorABGR;
typedef union { struct { u16 r:5; u16 g:6; u16 b:5; }; u16 val; } sColor565;
#else
typedef union { struct { u32 b:8; u32 g:8; u32 r:8; u32 a:8; }; u32 val; } sColorARGB;
typedef union { struct { u32 r:8; u32 g:8; u32 b:8; u32 a:8; }; u32 val; } sColorABGR;
typedef union { struct { u16 b:5; u16 g:6; u16 r:5; }; u16 val; } sColor565;
#endif

// Memory [R,G,B,A] on all platforms — byte-identical to SDL_Color, no #if needed
typedef union { struct { u32 r:8; u32 g:8; u32 b:8; u32 a:8; }; u32 val; } sColorRGBA32;

typedef enum {
    EPalARGB,    // SDL2, big-endian SDL1 32-bit
    EPalABGR,    // little-endian SDL1 32-bit
    EPalRGBA32,  // SDL_Color compatible — for 8-bit indexed surfaces
    EPal565      // Falcon 16-bit
} EPalFormat;

typedef struct {
    
    u8 *spritemem;

    u8 *physic;
    u8 *logic;
    u8 *physic_alloc;       // raw malloc base for physic (for free)
    u8 *logic_alloc;        // raw malloc base for logic  (for free)
    u32 buffer_alloc_size;  // sizeof(physic_alloc) == sizeof(logic_alloc)

    s16 logx1;
    s16 logx2;
    s16 logy1;
    s16 logy2;

    s8 numelem;
    u8 invert_x;

    s16 depx;
    s16 depy;
    s16 depz;
    
    s16 oldcx;
    s16 oldcy;
    s16 oldcz;

    s16 oldacx;
    s16 oldacy;
    s16 oldacz;
    
    s16 dkpalet[1024];
    u8 fdarkpal;

    s32 backdes;
    s32 backaddr;
    s16 backprof;
    s16 backx1;
    s16 backx2;
    s16 backy1;
    s16 backy2;
    s16 backlarg;
    u8 *backmap;            // background-cache render target (chunky: full-screen 8bpp shadow)
    u8 *bgcache_alloc;      // raw malloc base of the 8bpp shadow (for free); NULL when no cache
    u8 *wdraw;             // current sprite-blit destination: image.logic, or backmap while capturing

    s32 basesprite;
    u16 libsprit;
    s32 debsprit;
    s32 finsprit;
    s32 backsprite;
    s32 tvsprite;
    s32 texsprite;
    s32 atexsprite;
    
    s8 spag;
    s8 wpag;
    s16 pagdx;
    s16 pagdy;
    s16 pagcount;
    
    u8 sback;
    u8 wback;
    s8 cback;
    u8 pback;
    
    u16 inkcolor;
    u16 line_a_mode;
    
    u8 tvmode;

    u32 svpalet[256];
    u32 svpalet2[256];
    u32 tpalet[256];
    u32 mpalet[256];
    s32 dpalet[256 * 3];

    u32 *atpalet;
    u32 *ampalet;

    EPalFormat pal_format;

    u8 flinepal;
    s16 firstpal[64];
    s16 tlinepal[64];

    u8 palc;
    u8 palt;
    u8 palt0;
    u8 ftopal;
    u8 thepalet;
    u8 defpalet;

    s16 tabfen[640];
    s16 *ptabfen;

    s32 bufrvb;

    s32 mousflag;

    u8 *bufpack;

    u8 timing;
    s8 fmouse;
    s16 fonum;

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
    s16 fenlargw;
    s16 clipx1;
    s16 clipy1;
    s16 clipx2;
    s16 clipy2;
    s16 clipl;
    s16 cliph;
    u8 fclip;

    u8 fitroutine;
    u8 fphysic;
    u8 fphytolog;
    u8 fremouse;
    u8 fremap;
    u8 fremouse2;
    u8 vtiming;
    s16 savmouse[2];
    u32 savmouse2[8];
    u8 switchgo;
    u8 *wlogic;
    s16 wlogx1;
    s16 wlogx2;
    s16 wlogy1;
    s16 wlogy2;
    s16 wloglarg;
    s16 loglarg; // 0x50 for st
    u8 insid;
    
    // Robinsons Requiem

    s16 switchland;
    u8 fdoland;
    s16 landfenx1;
    s16 landfeny1;
    s16 landfenx2;
    s16 landfeny2;
    s16 landclipx1;
    s16 landclipy1;
    s16 landclipx2;
    s16 landclipy2;
    s16 landclipl;
    s16 landcliph;
    s16 vbarclipx1;
    s16 vbarclipy1;
    s16 vbarclipx2;
    s16 vbarclipy2;

    s16 scdirect;

    s16 vbarbot;
    s16 vbarlarg;
    s16 vbarx;
    s16 vbarmid;
    s16 skyposx;
    s16 skyposy;
    s16 skyleft;

    u8 ftstpix;
    s16 xtstpix;
    s16 ytstpix;
    s32 antstpix;
    s16 cxtstpix;
    s16 cytstpix;
    s16 cztstpix;
    s16 dtstpix;
    s16 ntstpix;
    s16 etstpix;
    
    s16 solpixy;
    s16 solh;
    
    s16 toppixy;
    s16 toph;
    s16 precx;
    
    u32 tlpix;
    u32 atalti;
    u32 atlpix;
    u32 atalias;
    u32 tlland;
    u32 atlland;
    u8 landone;
    s16 purey;
    s16 purey2;
    s16 fhorizon;
   
    s16 vdarkw;

    u32 tex_persp_step;  // per-pixel perspective step (DOS: u16 quotient, Falcon CD: ROL-encoded u32)
    s16 tex_hbase;       // horizontal texture base (DOS: per-row, Falcon CD: per-col +0x20)

    u32 mapscreen;
    
    u32 zoombid;
    u32 landnewad;
    u32 landdata;
    
    s16 signedx;
    s16 signedy;
    
    s16 vgamodulo;
    s16 bitmodulo;
    s16 bitlarg;

    s32 spritprof;
    s32 spritnext;
    
    s32 ztflowx;
    s32 ztflowy;

    u8 ddrawdist;  // when set, doubles terrain view distance beyond game default

#if ALIS_SDL_VER > 1
    // Enhanced 32-bit renderer
    u8 emode;               // 0 = classic 8-bit, 1 = enhanced 32-bit RGBA
    u8 stripdepth;          // current strip depth 0=near, 255=far (set per strip in doland)
    u8 *depthbuf;           // per-pixel depth buffer (terrgbaw * terrgbah bytes)
    u8 **depthrows;         // row pointer table: depthrows[y] → depth row start
    u8 *omask;              // 1 byte per display pixel: 0=use terrgba, 1=use 8-bit palette
    u32 fogcol;             // ARGB fog/sky color for distance blending
    u8 fogbeg;              // depth where fog begins (0-255)
    u8 fogend;              // depth where fog is fully opaque (0-255)
    u32 *terrgba;           // native 32-bit ARGB terrain buffer (malloc'd, NULL if not allocated)
    u16 terrgbaw;           // width of enhanced buffer
    u16 terrgbah;           // height of enhanced buffer
    u32 terrgbas;           // pixels per row
    u32 **terrgbarows;      // row pointer table: terrgbarows[y] → pixel row start
    u32 darkrgba[256 * 256];// pre-resolved dark table: darkrgba[dark_page<<8|texel] → ARGB
#endif
} sImage;

extern sImage image;

void inisprit(void);

void createlem(u16 *curidx, u16 *previdx);
u8 searchelem(u16 *curidx, u16 *previdx);
s8 searchtete(u16 *curidx, u16 *previdx);
void killelem(u16 *curidx, u16 *previdx);
u8 testnum(u16 *curidx);
u8 nextnum(u16 *curidx, u16 *previdx);

void put(u16 idx);
void put_char(s8 character);
void put_string(void);

void putin(u16 idx);
void putmapin(u16 spridx, s32 bitmap);
void scalaire_v10(s16 scene, s16 *x, s16 *y, s16 *z);
void picture_v10(u16 idx);

// clamp a filled box (x2 exclusive, y2 inclusive) to the screen, 0 when nothing is left
static inline int boxf_clip(s16 *x1, s16 *y1, s16 *x2, s16 *y2)
{
    if (*y1 > *y2) { s16 t = *y1; *y1 = *y2; *y2 = t; }
    if (*x1 < 0) *x1 = 0;
    if (*y1 < 0) *y1 = 0;
    if (*x2 > alis.platform.width) *x2 = alis.platform.width;
    if (*y2 >= alis.platform.height) *y2 = alis.platform.height - 1;
    return *x1 < *x2 && *y1 <= *y2;
}
void destofen(sSprite *sprite);

u32 itroutine(u32 interval, void *param);
void draw(void);

void draw_pixel(s16 x0, s16 y0);
void draw_line(s16 x0, s16 y0, s16 x1, s16 y1);
void draw_box(s16 x1,s16 y1,s16 x2,s16 y2);
void draw_boxf(s16 x1,s16 y1,s16 x2,s16 y2);
void trsfen(u8 *src, u8 *tgt);
void clrfen(void);

void topalette(u8 *paldata, s32 duration);
void toblackpal(s16 duration);

void savepal(s16 mode);
void restorepal(s16 mode, s32 duration);

// DOS palette model (PC data, see image.c); dos_pal_active() selects it.
int  dos_pal_active(void);
void dos_pal_reset(void);
void dos_pal_tick(void);
void dos_pal_sync_from_image(void);
void dos_cpalette(s16 idx, u8 *res);
void dos_ctopalet(s16 idx, s16 dur, u8 *res);
void dos_ctoblack(s16 dur);
void dos_cselpalet(s16 val);
void dos_cdefcolor(s16 idx, u16 val);
void dos_csavepal(s16 idx);
void dos_putin_palette(u8 *res);
int  dos_film_palette(u8 *addr);

void selpalet(void);
void setmpalet(void);

void setlinepalet(void);

#if defined(ALIS_TRACE_PAL)
void pal_trace(const char *op, s32 a, s32 b);   // debug: palette opcodes + bank/line-palette state
#define PAL_TRACE(op, a, b) pal_trace(op, a, b)
#else
#define PAL_TRACE(op, a, b) ((void)0)
#endif

s16 debprotf(s16 d2w);
u16 rangesprite(u16 elemidx1, u16 elemidx2, u16 elemidx3);

void valtostr(char *string, s16 value);

void mac_update_pos(s16 *x,s16 *y);

extern u8 cga_palette[16];
extern u8 masks[4];
extern u8 rots[4];

s32 calctop(u32 scene_addr, s16 grid_col, s16 grid_row);

void tvtofen(void);

#define PAL_WRITE_DISPATCH(pal, idx, body) do { \
    switch (image.pal_format) { \
    case EPalARGB:   { sColorARGB   *p = &((sColorARGB   *)(pal))[(idx)]; body; break; } \
    case EPalABGR:   { sColorABGR   *p = &((sColorABGR   *)(pal))[(idx)]; body; break; } \
    case EPalRGBA32: { sColorRGBA32 *p = &((sColorRGBA32 *)(pal))[(idx)]; body; break; } \
    case EPal565:    { sColor565    *p = &((sColor565    *)pal)[(idx)]; body; break; } \
    } \
} while(0)

#define RGB9(addr) { \
    r = (*(addr) & 0b00000111) << 5; addr++; \
    g = (*(addr) >> 4) << 5; \
    b = (*(addr) & 0b00000111) << 5;  addr++; \
}

#define RGB12(addr) { \
    r = (*(addr) & 0b00001111) << 4; addr++; \
    g = (*(addr) >> 4) << 4; \
    b = (*(addr) & 0b00001111) << 4;  addr++; \
}

#define RGB18(addr) { \
    r = *(addr) << 2; addr++; \
    g = *(addr) << 2; addr++; \
    b = *(addr) << 2; addr++; \
}

#define RGB24(addr) { \
    r = *(addr); addr++; \
    g = *(addr); addr++; \
    b = *(addr); addr++; \
}

#define RGB32(addr) { \
    r = *(addr); addr++; \
    g = *(addr); addr++; \
    b = *(addr); addr++; \
    addr++; \
}

#define PAL_WRITE_RGB(pal, index, red, grn, blu) \
switch (image.pal_format) { \
case EPalRGBA32: { sColorRGBA32 *cc = (sColorRGBA32 *)&(pal)[(index)]; cc->r = red; cc->g = grn; cc->b = blu; break; } \
case EPalABGR:   { sColorABGR *cc = (sColorABGR *)&(pal)[(index)]; cc->r = red; cc->g = grn; cc->b = blu; break; } \
case EPal565:    { sColor565 *cc = &((sColor565 *)(pal))[(index)]; cc->r = red >> 3; cc->g = grn >> 2; cc->b = blu >> 3; break; } \
default:         { sColorARGB *cc = (sColorARGB *)&(pal)[(index)]; cc->r = red; cc->g = grn; cc->b = blu; break; } \
}

#define PAL_WRITE_RGB_AT(pal, index, getter) { \
u8 r, g, b; \
switch (image.pal_format) { \
case EPalRGBA32: { getter; sColorRGBA32 *cc = (sColorRGBA32 *)&(pal)[(index)]; cc->r = r; cc->g = g; cc->b = b; break; } \
case EPalABGR:   { getter; sColorABGR *cc = (sColorABGR *)&(pal)[(index)]; cc->r = r; cc->g = g; cc->b = b; break; } \
case EPal565:    { getter; sColor565 *cc = &((sColor565 *)(pal))[(index)]; cc->r = r >> 3; cc->g = g >> 2; cc->b = b >> 3; break; } \
default:         { getter; sColorARGB *cc = (sColorARGB *)&(pal)[(index)]; cc->r = r; cc->g = g; cc->b = b; break; } \
}}

#define PAL_WRITE(pal, start, len, getter) {\
u8 r, g, b; \
switch (image.pal_format) { \
case EPalRGBA32: { for (int c = 0; c < (len); c++) { getter; sColorRGBA32 *cc = (sColorRGBA32 *)&(pal)[(start) + c]; cc->r = r; cc->g = g; cc->b = b; } break;} \
case EPalABGR:   { for (int c = 0; c < (len); c++) { getter; sColorABGR *cc = (sColorABGR *)&(pal)[(start) + c]; cc->r = r; cc->g = g; cc->b = b; } break;} \
case EPal565:    { for (int c = 0; c < (len); c++) { getter; sColor565 *cc = &((sColor565 *)(pal))[(start) + c]; cc->r = r >> 3; cc->g = g >> 2; cc->b = b >> 3; } break;} \
default:         { for (int c = 0; c < (len); c++) { getter; sColorARGB *cc = (sColorARGB *)&(pal)[(start) + c]; cc->r = r; cc->g = g; cc->b = b; } break;} \
}}

#define PAL_READ_RGB(pal, idx, _r, _g, _b) \
    switch (image.pal_format) { \
    case EPalARGB:   { sColorARGB   *p = &((sColorARGB   *)(pal))[(idx)]; _r=p->r; _g=p->g; _b=p->b; break; } \
    case EPalABGR:   { sColorABGR   *p = &((sColorABGR   *)(pal))[(idx)]; _r=p->r; _g=p->g; _b=p->b; break; } \
    case EPalRGBA32: { sColorRGBA32 *p = &((sColorRGBA32 *)(pal))[(idx)]; _r=p->r; _g=p->g; _b=p->b; break; } \
    case EPal565:    { sColor565    *p = &((sColor565    *)pal)[(idx)]; _r=p->r; _g=p->g; _b=p->b; break; } \
}

#define TOPALET_INTERP_LOOP(type, amp_ptr, atp_ptr) { \
    type *amp = (type *)(amp_ptr); \
    type *atp = (type *)(atp_ptr); \
    s32 _palc = image.palc; \
    for (int i = 0; i < 256; i++) { \
        if (amp[i].val != atp[i].val) { \
            amp[i].r = atp[i].r + (s32)((image.dpalet[i * 3 + 0] * _palc) >> 16); \
            amp[i].g = atp[i].g + (s32)((image.dpalet[i * 3 + 1] * _palc) >> 16); \
            amp[i].b = atp[i].b + (s32)((image.dpalet[i * 3 + 2] * _palc) >> 16); \
        } \
    } \
}

#define PAL_DELTA_LOOP(type, src_ptr, dst_ptr, duration) { \
    type *src = (type *)(src_ptr); \
    type *dst = (type *)(dst_ptr); \
    s32 _dur = (s32)(duration); \
    if (_dur == 0) _dur = 1; \
    for (int i = 0; i < 256; i++) { \
        image.dpalet[i * 3 + 0] = (((s32)src[i].r - (s32)dst[i].r) * 65536) / _dur; \
        image.dpalet[i * 3 + 1] = (((s32)src[i].g - (s32)dst[i].g) * 65536) / _dur; \
        image.dpalet[i * 3 + 2] = (((s32)src[i].b - (s32)dst[i].b) * 65536) / _dur; \
    } \
}

#if defined(ALIS_NATIVE_PLANAR)
void planar_tab_flush(void);   // drop cached flipped sprites (sprite_planar.c)
#endif
#if defined(ALIS_NATIVE_16BPP)
void sprite_cache_flush(void); // drop cached expanded sprites (image_draw_16.c)
#endif
#if defined(ALIS_RRQ_ASM_ZOOM) && ALIS_RRQ_ASM_ZOOM && !defined(ALIS_NO_ZOOM_TRIM)
void zoom_rows_flush(void);    // drop cached billboard row spans (render3d.c)
#endif
