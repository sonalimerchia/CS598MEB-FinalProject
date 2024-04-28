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

double** make_distance_matrix(size_t num_cells);
void save_distance_matrix(double** dist, char* filename, size_t num_cells);

int16_t max(int16_t a, int16_t b);

#endif // _UTILS_H_