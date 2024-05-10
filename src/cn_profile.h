#ifndef _CP_PROFILE_H_
#define _CP_PROFILE_H_

#include "utils.h"
#include "queue.h"

typedef struct cn_profile_data {
    size_t num_cells;
    size_t num_loci;;
    size_t num_chrom;
    loci_t B;
    queue_t* queue;
    cn_profile_t* profiles;
} cn_profile_data_t;

cn_profile_data_t* initialize_cp_profiles(size_t num_cells, size_t num_chrom, size_t num_loci);

void destroy_cp_profiles(cn_profile_data_t*);


#endif // _CP_PROFILE_H_