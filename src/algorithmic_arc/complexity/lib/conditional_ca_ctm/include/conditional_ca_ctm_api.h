#ifndef CONDITIONAL_CA_CTM_API_H
#define CONDITIONAL_CA_CTM_API_H

#include <stdint.h>  // For uint32_t, uint64_t

#ifdef __cplusplus
extern "C" {
#endif

void run_ctm(
    uint32_t* xs_flat,                 // Flattened input matrices
    uint32_t* ys_flat,                 // Flattened target matrices
    int num_pairs,                     // Number of (x, y) pairs
    int num_rules,                     // Number of rules to test
    unsigned int seed,                 // Random seed for rule generation
    int boundary_mode,                 // Boundary condition (e.g. toroidal or zero-padded)
    int max_steps,                     // Max simulation steps
    double* ms_out,                    // Output: m(Y|X) values
    double* ctms_out,                  // Output: CTM(Y|X) values
    uint64_t*** match_rule_numbers,   // Output: [num_pairs][match_counts[i]][8] 512-bit rule numbers
    int** match_rule_depths,          // Output: [num_pairs][match_counts[i]] logical depths
    int* match_counts                 // Output: [num_pairs] count of matched rules
);

#ifdef __cplusplus
}
#endif

#endif