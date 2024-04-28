#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "zcnt.h"

int16_t zcnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci) {
    int16_t* d1 = generate_delta(p1, num_loci);
    int16_t* d2 = generate_delta(p2, num_loci);

    int16_t sum = 0;
    for (size_t i = 0; i <= num_loci; ++i) {
        sum += abs(d1[i] - d2[i]);
    }
    sum /= 2;

    free(d1);
    d1 = NULL;
    free(d2);
    d2 = NULL;

    return sum;
}

int16_t* generate_delta(cn_profile_t* p_ptr, size_t num_loci) {
    cn_profile_t p = *p_ptr;
    int16_t* d = calloc(num_loci + 1, sizeof(int16_t));
    d[0] = 2 - p[0];

    for (size_t i = 1; i < num_loci; ++i) {
        d[i] = p[i] - p[i-1];
    }

    d[num_loci] = 2 - p[num_loci - 1];
    return d;
}