//
//  platform.h
//  alis
//

#ifndef platform_h
#define platform_h

#include "config.h"

typedef enum {
    EPlatformAtari = 0,
    EPlatformFalcon,
    EPlatformAmiga,
    EPlatformAmigaAGA,
    EPlatformMac,
    EPlatformPC,
    EPlatformUnknown
} EPlatform;

#define kMainScriptName ("main")



typedef struct {
    EPlatform   kind;
    char        desc[kDescMaxLen];  // platform description
    char        ext[8];             // script file extension
    u16         width;              // screen info
    u16         height;
    u8          bpp;
    char        path[kPathMaxLen];  // path to scripts
    char        main[kPathMaxLen];  // path to main script
} sPlatform;


sPlatform   guess_platform(const char * folder_path);
int         is_supported(sPlatform platform);

#endif /* platform_h */
