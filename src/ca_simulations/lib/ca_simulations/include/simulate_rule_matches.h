#ifndef SIMULATE_RULE_MATCHES_H
#define SIMULATE_RULE_MATCHES_H

#include <stdint.h>
#include "matrix_utils.h"  // includes Rule512, RULE_BYTES, Matrix

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
);

void free_matches(
    int num_pairs,
    int* match_counts,
    int** match_rule_depths,
    uint64_t*** match_rule_numbers
);

#endif  // SIMULATE_RULE_MATCHES_H
