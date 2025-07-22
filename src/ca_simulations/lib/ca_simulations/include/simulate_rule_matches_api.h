#ifndef SIMULATE_RULE_OUTPUTS_API_H
#define SIMULATE_RULE_OUTPUTS_API_H

#include <stdint.h>       // For uint32_t, uint64_t
#include "matrix_utils.h" // For Matrix definition

#ifdef __cplusplus
extern "C" {
#endif

#define RULE_UINT64_PARTS 8  // Each rule is 512 bits = 8 × uint64_t

/**
 * Struct to hold simulation outputs for a single CA rule.
 */
typedef struct {
    uint64_t rule_number[RULE_UINT64_PARTS];  // 512-bit CA rule ID
    Matrix* outputs;                          // Array of 4×4 binary matrices
    int* depths;                              // Simulation step for each output
    int num_outputs;                          // Number of unique outputs found
} OutputMap;

/**
 * Simulate each provided rule from a fixed input matrix, collecting reachable outputs
 * until a cycle is detected or max_steps is reached.
 *
 * @param x_flat             Flattened 4×4 input matrix (length 16)
 * @param rules_flat         Flattened rule array: num_rules × 8 uint64_t entries
 * @param num_rules          Number of rules (i.e. number of OutputMaps to produce)
 * @param boundary_mode      Boundary condition: 1 = toroidal, 0 = zero-padded
 * @param max_steps          Maximum simulation steps per rule
 * @param output_maps_out    Output: pointer to array of OutputMap[num_rules]
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
 *
 * @param num_rules       Number of OutputMap entries
 * @param output_maps     Array of OutputMap structs to be freed
 */
void free_output_maps(
    int num_rules,
    OutputMap* output_maps
);

#ifdef __cplusplus
}
#endif

#endif  // SIMULATE_RULE_OUTPUTS_API_H
