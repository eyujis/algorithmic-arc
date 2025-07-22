#ifndef SIMULATE_RULE_OUTPUTS_API_H
#define SIMULATE_RULE_OUTPUTS_API_H

#include <stdint.h>  // For uint32_t, uint64_t
#include "matrix_utils.h"  // For Matrix definition

#ifdef __cplusplus
extern "C" {
#endif

#define RULE_UINT64_PARTS 8

/**
 * Struct to hold simulation outputs for a single CA rule.
 * Each OutputMap contains:
 * - rule_number: 512-bit ID of the rule
 * - outputs:     list of matrices (each a 4x4 binary Matrix)
 * - depths:      step at which each output appeared
 * - num_outputs: number of outputs before cycle or interruption
 */
typedef struct {
    uint64_t rule_number[RULE_UINT64_PARTS];  // 512-bit CA rule ID
    Matrix* outputs;   // Dynamically allocated array of 4x4 output matrices
    int* depths;       // Matching array of steps when each output occurred
    int num_outputs;   // Number of outputs stored
} OutputMap;

/**
 * Simulate each rule from a fixed input matrix, collecting all reachable unique outputs
 * until a cycle is detected or max_steps is reached.
 *
 * @param x_flat             Flattened 4x4 input matrix (length 16)
 * @param num_rules          Number of random CA rules to simulate
 * @param seed               Random seed for rule generation
 * @param boundary_mode      Boundary condition: 1 = toroidal, 0 = zero-padded
 * @param max_steps          Maximum steps to simulate before cycle check stops
 * @param output_maps_out    Output: pointer to array of OutputMap (length = num_rules)
 */
void simulate_rule_outputs(
    uint32_t* x_flat,
    int num_rules,
    unsigned int seed,
    int boundary_mode,
    int max_steps,
    OutputMap** output_maps_out
);

/**
 * Frees memory allocated by simulate_rule_outputs.
 *
 * @param num_rules      Number of rules in output_maps
 * @param output_maps    Pointer to array of OutputMap structures
 */
void free_output_maps(
    int num_rules,
    OutputMap* output_maps
);

#ifdef __cplusplus
}
#endif

#endif  // SIMULATE_RULE_OUTPUTS_API_H
