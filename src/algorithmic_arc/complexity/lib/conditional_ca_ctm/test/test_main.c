#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "conditional_ca_ctm.h"

int main() {
    uint32_t xs_flat[2 * 16] = {
        // x0
        0,0,0,0,
        0,1,1,0,
        0,1,1,0,
        0,0,0,0,
        // x1
        0,0,1,0,
        0,1,0,1,
        1,0,1,0,
        0,0,1,0,
    };

    uint32_t ys_flat[2 * 16] = {
        // y0
        0,0,0,0,
        0,0,1,0,
        0,1,0,0,
        0,0,0,0,
        // y1
        0,0,0,0,
        0,1,1,0,
        0,1,1,0,
        0,0,0,0,
    };

    int num_pairs = 2;
    int num_rules = 100000;
    int boundary_mode = 1;
    int max_steps = 512;
    unsigned int seed = 42;

    double ms_out[2], ks_out[2];

    run_ctm(xs_flat, ys_flat, num_pairs, num_rules, seed, boundary_mode, max_steps, ms_out, ks_out);

    for (int i = 0; i < num_pairs; ++i) {
        printf("Pair %d: m=%.6f, k=%.6f\n", i, ms_out[i], ks_out[i]);
    }

    return 0;
}
