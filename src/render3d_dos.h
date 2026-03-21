//
//  render3d_dos.h
//  alis
//
//  DOS-specific terrain rendering functions extracted from render3d.c
//

#ifndef render3d_dos_h
#define render3d_dos_h

#include "config.h"

void vgatofen_dos(void);
void clrvga_dos(void);
void calclan0_dos(s32 scene_addr, s32 render_context);
void doland_dos(s32 scene_addr, s32 render_context);
void landtofi_dos(s16 unused, s16 scene_id);

#endif /* render3d_dos_h */
