#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h> 
#include <unistd.h> 
#include <stdbool.h>

#include "utils.h"
#include "cn_profile.h"
#include "read_csv.h"

cn_profile_data_t* read_cn_profiles(char* filepath) {
    if (!filepath) {
        return NULL; 
    }

    char* filepath_c = strdup(filepath);
    char* filename = basename(filepath_c); 
    LOG("[read_cn_profiles]: Read Basename (%s)", filename);

    size_t num_cells = 0;
    size_t num_loci = 0;
    int num_found = sscanf(filename, "n%zu_l%zu_s%*zu_full_cn_profiles.csv", &num_cells, &num_loci);

    free(filepath_c); 
    filepath_c = NULL; 

    num_cells *= 2;
    LOG("[read_cn_profiles]: Read %d Parameters: num_cells=(%zu), num_loci=(%zu)", num_found, num_cells, num_loci);
    if (num_found < 2 || !num_cells || !num_loci) {
        PRINT("File name not in expected format. Expected n{num_cells/2}_l{num_loci}_s{rand_seed}_full_cn_profiles.csv");
        return NULL;
    }

    FILE* csv_file = fopen(filepath, "r");
    if (!csv_file) {
        PRINT("Error opening csv file: %s", filepath);
        return NULL;
    }

    bool valid = verify_headers(csv_file);
    if (!valid) {
        PRINT("Invalid headers of data file. Expecting [node,chrom,start,end,cn_a]");
        return NULL;
    }

    cn_profile_data_t* data = initialize_cp_profiles(num_cells, num_loci);

    size_t node = 0;
    size_t chrom = 0;
    size_t start = 0;
    size_t end = 0;
    size_t copy_number = 0;
    size_t i = 0;
    while (!feof(csv_file)) {
        int num_read = fscanf(csv_file, "%zu,%zu,%zu,%zu,%zu\n", &node, &chrom, &start, &end, &copy_number);
        if (num_read == 0) {
            LOG("READ %d", num_read);
            continue;
        }

        // LOG("(%zu)(%zu)(%zu)(%zu)(%zu)", node, chrom, start, end, copy_number);
        if (chrom != 1) {
            LOG("NOT CURRENTLY SET UP FOR MULTIPLE CHROMOSOMES! ABORT!"); 
            destroy_cp_profiles(data); 
            return NULL;
        }

        for (size_t l = start; l <= end; ++l) {
            data->profiles[node][l] = copy_number;
            ++i;
        }
        data->B = max(data->B, copy_number);
    }
    LOG("[read_cn_profiles]: Read %zu copy numbers", i)
    return data;
}

bool verify_headers(FILE* csv_file) {
    char buf[50];
    int num_read = fscanf(csv_file, "%49s", buf);

    if (num_read < 1 || strcmp(buf, "node,chrom,start,end,cn_a")) {
        return false;
    }

    LOG("[verify_headers]: Correct headers!");
    return true;
}