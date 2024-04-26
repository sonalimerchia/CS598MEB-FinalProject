#ifndef _CP_PROFILE_H_
#define _CP_PROFILE_H_

#include "utils.h"

typedef struct cn_profile_data {
    size_t num_cells;
    size_t num_loci;
    cn_profile_t* profiles;
} cn_profile_data_t;

cn_profile_data_t* initialize_cp_profiles(size_t, size_t);

void destroy_cp_profiles(cn_profile_data_t*);


#endif // _CP_PROFILE_H_