#ifndef _CN3_H_
#define _CN3_H_

#include "cn_profile.h"
#include "utils.h"

int16_t solve_subproblem(int16_t* L, loci_t B, cn_profile_t u, cn_profile_t v, size_t i, loci_t m, loci_t du, loci_t au, loci_t dv, loci_t av);
int16_t cn3_distance(cn_profile_t* p1, cn_profile_t* p2, loci_t B, size_t num_loci);
size_t idx(loci_t B, size_t i, loci_t m, loci_t du, loci_t au, loci_t dv, loci_t av);

#endif // _CN3_H_