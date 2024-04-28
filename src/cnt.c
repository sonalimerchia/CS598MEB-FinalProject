#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "cnt.h"

double cnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci) {
    if (!is_reachable_cnt(p1, p2, num_loci)) return -1;
// (*p)[i]
    cn_profile_t S = *p1;
    cn_profile_t T = *p2;

    if (T[0] == 0 || T[num_loci - 1] == 0) {
        // Augment S and T
        cn_profile_t S_a = make_augmented_profile(p1, num_loci);
        cn_profile_t T_a = make_augmented_profile(p2, num_loci);

        // Get equivalent distance
        double dist = cnt_distance(&S_a, &T_a, num_loci + 2);

        // Free memory
        free(S_a);
        S_a = NULL;
        free(T_a);
        T_a = NULL;

        return dist;
    }



    for (size_t i = 0; i < 1; ++i) {
        if (T[i] == 0) {
            continue;
        }

        int16_t u_i = S[i] - T[i];
        int16_t di_min = max(u_i, 0);
        int16_t di_max = max(S[i] - 1, 0);

        LOG("di_min (%d), di_max (%d), u_i (%d)", u_i, di_min, di_max);
    }

    return 0;
}

cn_profile_t make_augmented_profile(cn_profile_t* P, size_t num_loci) {
    loci_t* P_a = calloc(num_loci + 2, sizeof(loci_t));
    memcpy(P_a + 1, P, num_loci * sizeof(loci_t));
    P_a[0] = 1;
    P_a[num_loci + 1] = 1;
    return (cn_profile_t)P_a;
}

bool is_reachable_cnt(cn_profile_t* S, cn_profile_t* T, size_t num_loci) {
    for (size_t i = 0; i < num_loci; ++i) {
        if (S[i] == 0 && T[i] > 0) return false;
    } 

    return true;
}