#include <stdlib.h>

#include "cn3.h"
#include "utils.h"
#include "cn_profile.h"

int16_t cn3_distance(cn_profile_t* p1, cn_profile_t* p2, loci_t e, size_t num_loci) {
    LOG("[cn3_distance]: e=%hu", e);
    size_t n = num_loci;

    chrom_t u = (*p1)[0];
    chrom_t v = (*p2)[0];

    loci_t B = umax(array_max(u, n), array_max(v, n));
    LOG("CN3 n=%zu, B=%hu", num_loci, B);

    int16_t* L = calloc(n * (B + 1) * (B + 1) * (B + 1) * (B + 1) * (B + 1), sizeof(int16_t));
    for (size_t i = 0; i < n; ++i) {
        for (loci_t m = 0; m <= B; ++m) {
            for (loci_t du = 0; du <= B; ++du) {
                for (loci_t au = 0; au <= B; ++au) {
                    for (loci_t dv = 0; dv <= B; ++dv) {
                        for (loci_t av = 0; av <= B; ++av) {
                            L[idx(B, i, m, du, au, dv, av)] = solve_subproblem(
                                L, B, u, v,
                                i, m, du, au, dv, av
                            );
                        }
                    }
                }
            }
        }
    }

    int16_t dist = -1;
    for (loci_t mp = 0; mp <= B; ++mp) {
        for (loci_t dup = 0; dup <= B; ++dup) {
            for (loci_t aup = 0; aup <= B; ++aup) {
                for (loci_t dvp = 0; dvp <= B; ++dvp) {
                    for (loci_t avp = 0; avp <= B; ++avp) {
                        dist = inf_min(dist, L[(idx(B, n-1, mp, dup, aup, dvp, avp))]);
                    }
                }
            }
        }
    }

    free(L);
    L = NULL;
    return dist;
}

size_t idx(loci_t B, size_t i, loci_t m, loci_t du, loci_t au, loci_t dv, loci_t av) {
    size_t B1 = B+1;
    size_t B2 = B1 * B1;
    size_t B3 = B2 * B1;
    size_t B4 = B3 * B1;
    size_t B5 = B4 * B1;

    return i * B5 + m * B4 + du * B3 + au * B2 + dv * B1 + av;
}

int16_t solve_subproblem(int16_t* L, loci_t B, chrom_t u, chrom_t v, size_t i, loci_t m, loci_t du, loci_t au, loci_t dv, loci_t av) {
    int16_t mi = m;
    int16_t ui = u[i];
    int16_t vi = v[i];

    if (ui > 0 && (du >= mi || ui != mi - du + au)) {
        return -1;
    } else if (vi > 0 && (dv >= mi || vi != mi - dv + av)) {
        return -1;
    } else if (ui == 0 && du < mi) {
        return -1;
    } else if (vi == 0 && dv < mi) {
        return -1;
    } else if (i == 0) {
        return du + au + dv + av;
    }

    int16_t dist = -1;
    for (loci_t mp = 0; mp <= B; ++mp) {
        for (loci_t dup = 0; dup <= B; ++dup) {
            for (loci_t aup = 0; aup <= B; ++aup) {
                for (loci_t dvp = 0; dvp <= B; ++dvp) {
                    for (loci_t avp = 0; avp <= B; ++avp) {
                        int16_t opp = L[(idx(B, i-1, mp, dup, aup, dvp, avp))];
                        if (opp == -1) {
                            continue;
                        }

                        opp += unsigned_sub(du, dup);
                        opp += unsigned_sub(au, aup);
                        opp += unsigned_sub(dv, dvp);
                        opp += unsigned_sub(av, avp);
                        dist = inf_min(dist, opp);
                    }
                }
            }
        }
    }

    return dist;
}