#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "utils.h"

int16_t** make_distance_matrix(size_t num_cells) {
    int16_t** matrix = calloc(sizeof(int16_t*), num_cells);

    for (size_t i = 0; i < num_cells; ++i) {
        matrix[i] = calloc(sizeof(int16_t), num_cells);
    }

    return matrix; 
}

void save_distance_matrix(int16_t** dist, const char* output_prefix, const char* filename, size_t num_cells) {
    char* filepath = concatenate_filename(output_prefix, filename);
    LOG("[save_distance_matrix]: Saving distance matrix to %s", filepath);

    // Open destination file
    FILE* file = fopen(filepath, "w");
    if (!file) {
        perror("[save_distance_matrix]:");
        free(filepath);
        filepath = NULL;
        return;
    }

    // Write data
    for (size_t i = 0; i < num_cells; ++i) {
        fprintf(file, "%hd", dist[i][0]);
        for (size_t j = 1; j < num_cells; ++j) {
            fprintf(file, ",%hd", dist[i][j]);
        }
        fprintf(file, "\n");
    }

    // Close file
    int err = fclose(file);
    if (err) {
        PRINT("An error occurred while writing distance matrix to %s", filepath);
    }
    
    free(filepath);
    filepath = NULL;
}

void destroy_distance_matrix(int16_t** dist, size_t num_cells) {
    for (size_t i = 0; i < num_cells; ++i) {
        free(dist[i]);
        dist[i] = NULL;
    }

    free(dist);
    dist = NULL;
}

void save_time_data(time_t* times, const char* output_prefix, const char* filename, size_t num_times) {
    char* filepath = concatenate_filename(output_prefix, filename);
    LOG("[save_time_data]: Saving time data to %s", filepath);

    // Open destination file
    FILE* file = fopen(filepath, "w");
    if (!file) {
        perror("[save_time_data]:");

        free(filepath);
        filepath = NULL;
        return;
    }

    fprintf(file, "%lu", times[0]);
    for (size_t i = 1; i < num_times; ++i) {
        fprintf(file, ",%lu", times[i]);
    }

    // Close file
    int err = fclose(file);
    if (err) {
        PRINT("An error occurred while writing distance matrix to %s", filename);
    }

    free(filepath);
    filepath = NULL;
}

/** From https://stackoverflow.com/a/380446 */
int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p) {
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

int16_t max(int16_t a, int16_t b) {
    if (a > b) {
        return a;
    }

    return b;
}

int16_t min(int16_t a, int16_t b) {
    if (a < b) {
        return a;
    }

    return b;
}

char* concatenate_filename(const char* prefix, const char* suffix) {
    size_t p_len = strlen(prefix);
    size_t s_len = strlen(suffix);

    char* filename = calloc(p_len + s_len + 2, sizeof(char)); 
    memcpy(filename, prefix, p_len); 
    filename[p_len] = '-';
    memcpy(filename + p_len + 1, suffix, s_len);
    return filename;
}