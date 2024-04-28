#ifndef _CNT_H_
#define _CNT_H_

#include <stdbool.h>

#include "utils.h"

int16_t cnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci);

cn_profile_t make_augmented_profile(cn_profile_t* P, size_t num_loci);

bool is_reachable_cnt(cn_profile_t* S, cn_profile_t* T, size_t num_loci);

#endif // _CNT_H_