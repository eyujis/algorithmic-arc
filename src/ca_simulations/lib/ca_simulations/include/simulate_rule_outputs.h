#ifndef SIMULATE_RULE_OUTPUTS_H
#define SIMULATE_RULE_OUTPUTS_H

#include <stdint.h>
#include "matrix_utils.h"  // for Matrix

#define RULE_UINT64_PARTS 8  // 512-bit rule = 8 x uint64_t

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t rule_number[RULE_UINT64_PARTS];  // 512-bit CA rule ID
    Matrix* outputs;       // Array of output matrices
    int* depths;           // Corresponding step/depth for each output
    int num_outputs;       // Number of outputs found
} OutputMap;

/**
 * Simulate each provided rule from a single input x and collect all reachable unique outputs.
 *
 * @param x_flat             Flattened 4×4 binary matrix (input)
 * @param rules_flat         Flat array of rules (num_rules × 8 uint64_t blocks)
 * @param num_rules          Number of rules
 * @param boundary_mode      1 = toroidal, 0 = zero-padded
 * @param max_steps          Max simulation steps
 * @param output_maps_out    Output: array of OutputMap[num_rules]
 */
void simulate_rule_outputs(
    uint32_t* x_flat,
    uint64_t* rules_flat,
    int num_rules,
    int boundary_mode,
    int max_steps,
    OutputMap** output_maps_out
);

/**
 * Free memory allocated by simulate_rule_outputs.
 */
void free_output_maps(
    int num_rules,
    OutputMap* output_maps
);

#ifdef __cplusplus
}
#endif

#endif  // SIMULATE_RULE_OUTPUTS_H
