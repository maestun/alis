//
//  debug.c
//  alis
//
//  Created by developer on 10/09/2020.
//  Copyright © 2020 Zlot. All rights reserved.
//

#include "debug.h"

void debug(EDebugLevel level, char * format, ...) {
//    static FILE * dbfp = NULL;
//    if(dbfp == NULL) {
//        dbfp = fopen("alis_debug.log", "w");
//    }
    if(level <= DEBUG_LEVEL) {
        va_list arg;
        va_start(arg, format);
        vfprintf(stdout, format, arg);
        fflush(stdout);
        va_end(arg);
    }
}
