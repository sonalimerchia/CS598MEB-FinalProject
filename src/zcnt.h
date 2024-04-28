#ifndef _ZCNT_H_
#define _ZCNT_H_

#include "utils.h"

int16_t zcnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci);

int16_t* generate_delta(cn_profile_t* p, size_t num_loci);

#endif // _ZCNT_H_