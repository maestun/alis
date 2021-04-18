//
//  sys.h
//  alis
//

#ifndef sys_h
#define sys_h

#include "../config.h"
#include "../debug.h"

// =============================================================================
#pragma mark - LIFECYCLE
// =============================================================================
typedef void (*vmStep)(void);
void    sys_main(vmStep fStep);
void    sys_init(void);
u8      sys_poll_event(void);
void    sys_deinit(void);


// =============================================================================
#pragma mark - I/O
// =============================================================================
typedef struct {
    int x, y;
    u8 lb, rb;
    u8 enabled;
} mouse_t;
mouse_t sys_get_mouse(void);
void    sys_set_mouse(u16 x, u16 y);
void    sys_enable_mouse(u8 enable);


// =============================================================================
#pragma mark - GFX
// =============================================================================
typedef struct {
    u16     w, h;
    u8      scale;
    u8 *    data;
} pixelbuf_t;
void    sys_render(pixelbuf_t buffer);


// =============================================================================
#pragma mark - FILE SYSTEM
// =============================================================================
int     sys_fclose(FILE * fp);
FILE *  sys_fopen(char * path);
u8      sys_fexists(char * path);


// =============================================================================
#pragma mark - MISC
// =============================================================================
void    sys_set_time(u16 h, u16 m, u16 s);
time_t  sys_get_time(void);
u16     sys_get_model(void);
u16     sys_random(void);


#endif /* sys_h */
