#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "cnt.h"

int16_t cnt_distance(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci) {
    if (!is_reachable_cnt(p1, p2, num_loci)) return -1;

    cn_profile_t S = *p1;
    cn_profile_t T = *p2;

    if (T[0] == 0 || T[num_loci - 1] == 0) {
        // Augment S and T
        cn_profile_t S_a = make_augmented_profile(p1, num_loci);
        cn_profile_t T_a = make_augmented_profile(p2, num_loci);

        // Get equivalent distance
        int16_t dist = cnt_distance(&S_a, &T_a, num_loci + 2);

        // Free memory
        free(S_a);
        S_a = NULL;
        free(T_a);
        T_a = NULL;

        return dist;
    }

    int16_t u_j = S[0] - T[0];
    int16_t d1_min = max(u_j, 0);

    int16_t base_j = 2 * d1_min - u_j;
    int16_t a_j = d1_min;
    int16_t b_j = d1_min;
    int16_t Q_i = 0;

    for (size_t i = 1; i < num_loci; ++i) {
        if (T[i] == 0) {
            if (S[i] > Q_i) {
                Q_i = S[i];
            }
            continue;
        }

        int16_t u_i = S[i] - T[i];
        int16_t R_i = u_j - u_i;
        int16_t di_min = max(u_i, 0);
        int16_t di_max = max(S[i] - 1, 0);

        int16_t a_i, b_i;
        if (R_i >= 0) {
            if (Q_i <= a_j) {
                a_i = a_j - R_i; 
                b_i = b_j;
            } else if (a_j < Q_i && Q_i <= b_j) {
                a_i = Q_i - R_i;
                b_i = b_j;
            } else {
                a_i = b_j - R_i;
                b_i = Q_i;
            }
        } else {
            if (Q_i <= a_j) {
                a_i = a_j;
                b_i = b_j - R_i;
            } else if (a_j < Q_i && Q_i <= b_j) {
                a_i = Q_i;
                b_i = b_j - R_i;
            } else {
                a_i = min(Q_i, b_j - R_i);
                b_i = max(Q_i, b_j - R_i);
            }
        }

        int16_t base_i = base_j + max(Q_i - a_j, 0);
        if (a_i < di_min && di_min <= b_i) {
            base_i += di_min - a_i;
        } else if (b_i < di_min && di_min <= di_max) {
            base_i += 2 * di_min - a_i - b_i;
        }

        a_i = max(di_min, min(a_i, di_max));
        b_i = max(a_i, min(b_i, di_max));

        u_j = u_i;
        base_j = base_i;
        a_j = a_i; 
        b_j = b_i;
        Q_i = 0;
    }

    return base_j;
}

cn_profile_t make_augmented_profile(cn_profile_t* P, size_t num_loci) {
    loci_t* P_a = calloc(num_loci + 2, sizeof(loci_t));
    memcpy(P_a + 1, P, num_loci * sizeof(loci_t));
    P_a[0] = 1;
    P_a[num_loci + 1] = 1;
    return (cn_profile_t)P_a;
}

bool is_reachable_cnt(cn_profile_t* p1, cn_profile_t* p2, size_t num_loci) {
    cn_profile_t S = *p1;
    cn_profile_t T = *p2;
    for (size_t i = 0; i < num_loci; ++i) {
        if (S[i] == 0 && T[i] > 0) return false;
    } 

    return true;
}