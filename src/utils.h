#ifndef _UTILS_H_ 
#define _UTILS_H_

#include <stdio.h>
#include <time.h>

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

int16_t** make_distance_matrix(size_t num_cells);
void save_distance_matrix(int16_t** dist, const char* filename, size_t num_cells);

void save_time_data(time_t* times, const char* filename, size_t num_times);

int16_t max(int16_t a, int16_t b);
int16_t min(int16_t a, int16_t b);
int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);

#endif // _UTILS_H_