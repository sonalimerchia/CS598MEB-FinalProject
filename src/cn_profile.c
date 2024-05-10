#include <stdlib.h>

#include "cn_profile.h"
#include "utils.h"

cn_profile_data_t* initialize_cp_profiles(size_t num_cells, size_t num_chrom, size_t num_loci) {
    cn_profile_data_t* data = calloc(1, sizeof(cn_profile_data_t));

    data->num_cells = num_cells;
    data->num_chrom = num_chrom;
    data->num_loci = num_loci;
    data->profiles = calloc(num_cells, sizeof(cn_profile_t));

    for (size_t p = 0; p < num_cells; ++p) {
        data->profiles[p] = calloc(num_chrom, sizeof(chrom_t));
        for (size_t j = 0; j < num_chrom; ++j) {
            data->profiles[p][j] = calloc(num_loci, sizeof(loci_t));
        }
    }

    return data;
}

void destroy_cp_profiles(cn_profile_data_t* data) {
    LOG("[destroy_cp_profiles]");
    for (size_t p = 0; p < data->num_cells; p++) {
        for (size_t j = 0; j < data->num_cells; j++) {
            free(data->profiles[p][j]);
            data->profiles[p][j] = NULL;
        }
        free(data->profiles[p]);
        data->profiles[p] = NULL;
    }

    free(data->profiles);
    data->profiles = NULL;

    free(data);
    data = NULL;
}