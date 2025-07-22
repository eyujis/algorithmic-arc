#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <matrix_utils.h>
#include <simulate_rule_matches.h>

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

    int* match_rule_depths[NUM_PAIRS];
    uint64_t** match_rule_numbers[NUM_PAIRS];
    int match_counts[NUM_PAIRS];

    simulate_rule_matches(
        (uint32_t*)xs_flat,
        (uint32_t*)ys_flat,
        NUM_PAIRS,
        NUM_RULES,
        42,     // random seed
        1,      // toroidal boundary
        65536,    // max steps
        match_rule_numbers,
        match_rule_depths,
        match_counts
    );

    for (int i = 0; i < NUM_PAIRS; ++i) {
        printf("Pair %d: matches = %d\n", i, match_counts[i]);

        for (int j = 0; j < match_counts[i] && j < 3; ++j) { // Print up to 3 matches
            printf("  Match %d: depth = %d, rule = 0x", j, match_rule_depths[i][j]);
            for (int k = RULE_UINT64_PARTS - 1; k >= 0; --k) {
                printf("%016llx", (unsigned long long)match_rule_numbers[i][j][k]);
            }
            printf("\n");
        }
    }

    // Free rule number memory and depth arrays
    free_matches(NUM_PAIRS, match_counts, match_rule_depths, match_rule_numbers);

    return 0;
}
