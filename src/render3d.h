//
//  render3d.h
//  alis
//
//  Created by Vadim Kindl on 04.10.2025.
//  Copyright © 2025 Zlot. All rights reserved.
//

#ifndef render3d_h
#define render3d_h

void openland(s16 d0w);
void pointpix(s16 x, s16 y);

void landtofi(s16 d2w, s16 d3w);
void landtopix(s32 sceneadr, u16 d1w);
void affiland(s32 scene);

#endif /* render3d_h */
