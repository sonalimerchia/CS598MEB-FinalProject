#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "utils.h"

int16_t** make_distance_matrix(size_t num_cells) {
    int16_t** matrix = calloc(sizeof(int16_t*), num_cells);

    for (size_t i = 0; i < num_cells; ++i) {
        matrix[i] = calloc(sizeof(int16_t), num_cells);
    }

    return matrix; 
}

void save_distance_matrix(int16_t** dist, const char* filename, size_t num_cells) {
    LOG("[save_distance_matrix]: Saving distance matrix to %s", filename);

    // Open destination file
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("[save_distance_matrix]:");
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
        PRINT("An error occurred while writing distance matrix to %s", filename);
    }
}

void save_time_data(time_t* times, const char* filename, size_t num_times) {
    LOG("[save_time_data]: Saving time data to %s", filename);

    // Open destination file
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("[save_time_data]:");
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