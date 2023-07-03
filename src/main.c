//
//  main.c
//  alis
//
#include "alis.h"
#include "config.h"
#include "sys/sys.h"

void usage(void) {
    printf("%s v%s\nUsage:\n\t%s <data_path>\n\n\t%s <script_path>\n",
           kProgName, kProgVersion, kProgName, kProgName);
}


int main(int argc, const char* argv[]) {

    if (argc < 2) {
        usage();
    }
    else {
        sPlatform *pl = pl_guess(argv[1]);
        if(pl_supported(pl)) {
            sys_init();
            alis_init(*pl);
            
            alis_main();
            
            alis_deinit();
            sys_deinit();
        }
        else {
            debug(EDebugFatal,
                  "Platform '%s' is not supported.\n",
                  pl->desc);
        }
    }
    return 0;
}
