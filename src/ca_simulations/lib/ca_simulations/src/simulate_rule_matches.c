#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <prng/prng.h>
#include <prng/splitmix64.h>
#include <prng/xoshiro256plusplus.h>

#include "interrupt_flag.h"
#include "matrix_utils.h"
#include "ca_dynamics.h"
#include "simulate_rule_matches.h"

#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 65536
#endif

#define RULE_BITS 512
#define RULE_UINT64_PARTS (RULE_BITS / 64)

void simulate_rule_matches(
    uint32_t* xs_flat,
    uint32_t* ys_flat,
    int num_pairs,
    int num_rules,
    unsigned int seed,
    int boundary_mode,
    int max_steps,
    uint64_t*** match_rule_numbers,
    int** match_rule_depths,
    int* match_counts
) {
    init_interrupt_flag();

    if (max_steps <= 0) max_steps = DEFAULT_MAX_STEPS;
    prng_seed(seed);

    Rule512* rules = calloc(num_rules, sizeof(Rule512));
    if (!rules) {
        fprintf(stderr, "Memory allocation failed for rules.\n");
        exit(EXIT_FAILURE);
    }

    for (int r = 0; r < num_rules; ++r) {
        random_rule(&rules[r]);
    }

    Matrix x, y;

    for (int i = 0; i < num_pairs; ++i) {
        if (is_interrupted()) break;

        flat_to_matrix(x, &xs_flat[i * 16]);
        flat_to_matrix(y, &ys_flat[i * 16]);

        match_counts[i] = 0;
        match_rule_depths[i] = calloc(num_rules, sizeof(int));
        match_rule_numbers[i] = calloc(num_rules, sizeof(uint64_t*));

        if (!match_rule_depths[i] || !match_rule_numbers[i]) {
            fprintf(stderr, "Memory allocation failed for match tracking.\n");
            exit(EXIT_FAILURE);
        }

        for (int r = 0; r < num_rules; ++r) {
            int depth = simulate_with_depth(x, y, &rules[r], boundary_mode, max_steps);
            if (depth >= 0) {
                int idx = match_counts[i]++;
                match_rule_depths[i][idx] = depth;

                match_rule_numbers[i][idx] = calloc(RULE_UINT64_PARTS, sizeof(uint64_t));
                if (!match_rule_numbers[i][idx]) {
                    fprintf(stderr, "Memory allocation failed for rule number.\n");
                    exit(EXIT_FAILURE);
                }

                compute_rule_number(&rules[r], match_rule_numbers[i][idx]);
            }
        }
    }

    free(rules);

    if (is_interrupted()) {
        fprintf(stderr, "Interrupted by user (SIGINT).\n");
    }
}

void free_matches(
    int num_pairs,
    int* match_counts,
    int** match_rule_depths,
    uint64_t*** match_rule_numbers
) {
    for (int i = 0; i < num_pairs; ++i) {
        for (int j = 0; j < match_counts[i]; ++j) {
            free(match_rule_numbers[i][j]);
        }
        free(match_rule_numbers[i]);
        free(match_rule_depths[i]);
    }
}
