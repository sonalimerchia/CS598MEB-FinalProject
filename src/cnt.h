#ifndef _CNT_H_
#define _CNT_H_

#include <stdbool.h>

#include "utils.h"

int16_t cnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_chrom, size_t num_loci);

int16_t cnt_chrom_distance(chrom_t p1, chrom_t p2, size_t num_loci);

chrom_t make_augmented_profile(chrom_t P, size_t num_loci);

bool is_reachable_cnt(cn_profile_t* p1, cn_profile_t* p2, size_t num_chrom, size_t num_loci);

#endif // _CNT_H_