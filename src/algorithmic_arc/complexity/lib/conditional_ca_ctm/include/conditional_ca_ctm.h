#ifndef CONDITIONAL_ECA_CTM_H
#define CONDITIONAL_ECA_CTM_H

#include <stdint.h>

void run_ctm(
    uint32_t* xs_flat,
    uint32_t* ys_flat,
    int num_pairs,
    int num_rules,
    unsigned int seed,
    int boundary_mode,
    int max_steps,
    double* ms_out,
    double* ctms_out,
    int** match_rule_indices,  // [num_pairs][match_counts[i]]
    int** match_rule_depths,   // [num_pairs][match_counts[i]]
    int* match_counts          // [num_pairs] — how many rules matched for each pair
);

#endif
