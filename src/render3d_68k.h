//
//  render3d_68k.h
//  alis
//
//  m68k-specific terrain rendering functions extracted from render3d.c
//

#ifndef render3d_68k_h
#define render3d_68k_h

#include "config.h"

void vgatofen_68k(void);
void clrvga_68k(void);
void calclan0_68k(s32 scene_addr, s32 render_context);
void doland_68k(s32 scene_addr, s32 render_context);
void landtofi_68k(s16 unused, s16 scene_id);

#endif /* render3d_68k_h */
