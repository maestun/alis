//
//  Ishar 1 map flipper
//  Input: .fic map created by appending cont*.fic files.
//  Output: more readable map file.

#include <stdio.h>

#define NS_LEN 90
#define WE_LEN 324

int main(int argc, const char * argv[]) {
    
    if(argc != 3) {
        printf("Usage: contflip <inputfile> <outputfile>\n");
        return -1;
    }
    
    FILE * ohnd = fopen(argv[2], "wb+");
    if(!ohnd) {
        printf("Cannot create output file.\n");
        return -1;
    }

    FILE * ihnd = fopen(argv[1], "rb");
    if(!ihnd) {
        printf("Cannot open input file.\n");
        return -1;
    }
    
    for (unsigned int we = 0; we < WE_LEN; we++) {
        for (unsigned int ns = 0; ns < NS_LEN; ns++) {
            fputc(0, ohnd);
        }
    }

    unsigned char byte = 0;
    for (unsigned int we = 0; we < WE_LEN; we++) {
        for (unsigned int ns = 0; ns < NS_LEN; ns++) {
            
            // get input
            fseek(ihnd, ns + (we * NS_LEN), 0);
            fread(&byte, 1, 1, ihnd);
            
            // write output
            fseek(ohnd, we + (ns * WE_LEN), 0);
            fputc(byte, ohnd);
        }
    }
    
    fclose(ihnd);
    fclose(ohnd);
    return 0;
}
