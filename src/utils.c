#include <stdlib.h>
#include "utils.h"

double** make_distance_matrix(size_t num_cells) {
    double** matrix = calloc(sizeof(double*), num_cells);

    for (size_t i = 0; i < num_cells; ++i) {
        matrix[i] = calloc(sizeof(double), num_cells);
    }

    return matrix; 
}

void save_distance_matrix(double** dist, char* filename, size_t num_cells) {
    
}

int16_t max(int16_t a, int16_t b) {
    if (a > b) {
        return a;
    }

    return b;
}