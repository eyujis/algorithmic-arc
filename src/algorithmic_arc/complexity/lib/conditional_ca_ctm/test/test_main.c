#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <matrix_utils.h>
#include <conditional_ca_ctm.h>

#define NUM_PAIRS 2
#define NUM_RULES 1000000
#define RULE_UINT64_PARTS 8

int main() {
    uint32_t xs_flat[NUM_PAIRS][16] = {
        {0,0,0,0,
         0,1,1,0,
         0,1,1,0,
         0,0,0,0},

        {0,0,0,1,
         0,1,0,0,
         0,0,1,0,
         1,0,0,0}
    };

    uint32_t ys_flat[NUM_PAIRS][16] = {
        {0,0,0,0,
         1,1,1,0,
         1,1,1,0,
         0,0,0,0},

        {0,0,0,0,
         0,0,1,0,
         0,1,0,0,
         0,0,0,0}
    };

    double ms_out[NUM_PAIRS];
    double ctms_out[NUM_PAIRS];

    int* match_rule_depths[NUM_PAIRS];
    uint64_t** match_rule_numbers[NUM_PAIRS];
    int match_counts[NUM_PAIRS];

    run_ctm(
        (uint32_t*)xs_flat,
        (uint32_t*)ys_flat,
        NUM_PAIRS,
        NUM_RULES,
        42,
        1,      // toroidal boundary
        100,    // max steps
        ms_out,
        ctms_out,
        match_rule_numbers,
        match_rule_depths,
        match_counts
    );

    for (int i = 0; i < NUM_PAIRS; ++i) {
        printf("Pair %d: m = %.6f, CTM = %.6f, matches = %d\n", i, ms_out[i], ctms_out[i], match_counts[i]);
        // for (int j = 0; j < match_counts[i] && j < 3; ++j) { // print up to 3 matches
        //     printf("  Match %d: depth = %d, rule = 0x", j, match_rule_depths[i][j]);
        //     for (int k = RULE_UINT64_PARTS - 1; k >= 0; --k) {
        //         printf("%016llx", (unsigned long long)match_rule_numbers[i][j][k]);
        //     }
        //     printf("\n");
        // }

        // Free rule number memory and depth array
        for (int j = 0; j < match_counts[i]; ++j) {
            free(match_rule_numbers[i][j]);
        }
        free(match_rule_numbers[i]);
        free(match_rule_depths[i]);
    }

    return 0;
}
