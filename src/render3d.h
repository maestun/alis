//
//  render3d.h
//  alis
//
//  Created by Vadim Kindl on 04.10.2025.
//  Copyright © 2025 Zlot. All rights reserved.
//

#ifndef render3d_h
#define render3d_h

void openland(s16 scene_id);
void pointpix(s16 x, s16 y);

void landtofi(s16 unused, s16 scene_id);
void landtopix(s32 scene_addr, u16 sprite_id);
void affiland(s32 scene);

#endif /* render3d_h */
