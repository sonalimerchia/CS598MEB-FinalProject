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

    char* node = NULL;
    size_t chrom = 0;
    size_t start = 0;
    size_t end = 0;
    loci_t copy_number = 0;
    size_t i = 0;
    size_t node_idx = -1; 
    size_t locus = 0;
    while (!feof(csv_file)) {
        char* line = NULL;
        size_t num = 0;
        getline(&line, &num, csv_file);
        char* token = strtok(line, ",");
        LOG("%s", node)
        if (!node || strcmp(node, token)) {
            node_idx += 1;
            locus = 0;
            free(node);
            node = strdup(token);
            LOG("[read_csv]: Reading node %zu (%s)", node_idx, token);
        } 

        int num_read = 0;
        token = strtok(NULL, ",");
        num_read += sscanf(token, "%zu", &chrom);
        token = strtok(NULL, ",");
        num_read += sscanf(token, "%zu", &start);
        token = strtok(NULL, ",");
        num_read += sscanf(token, "%zu", &end);
        token = strtok(NULL, ",");
        num_read += sscanf(token, "%hu", &copy_number);

        if (num_read < 4) {
            continue;
        } 

        data->profiles[node_idx][locus] = copy_number;
        data->B = max(data->B, copy_number);
        ++i;
        ++locus;

        free(line); 
        line = NULL;
    }
    free(node);
    node = NULL; 

    LOG("[read_cn_profiles]: Read %zu copy numbers", i)
    return data;
}

bool verify_headers(FILE* csv_file) {
    char* s = NULL;
    size_t n = 0;
    getline(&s, &n, csv_file);
    free(s);

    return true;
}