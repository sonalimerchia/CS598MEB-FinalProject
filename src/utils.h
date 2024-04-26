#ifndef _UTILS_H_ 
#define _UTILS_H_

#include <stdio.h>

#define DEBUG 1
#define LOG(...)\
if (DEBUG) {\
    fprintf(stderr, __VA_ARGS__);\
    fprintf(stderr, "\n");\
}

#define PRINT(...) {\
    fprintf(stdout, __VA_ARGS__);\
    fprintf(stdout, "\n");\
}

typedef u_int16_t loci_t;
typedef loci_t* cn_profile_t;

int smile();

#endif // _UTILS_H_