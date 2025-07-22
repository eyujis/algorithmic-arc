#ifndef CA_DYNAMICS_H
#define CA_DYNAMICS_H

#include <stdint.h>
#include "matrix_utils.h"  // defines Rule512, Matrix, MATRIX_SIZE, RULE_BYTES

#ifdef __cplusplus
extern "C" {
#endif

uint16_t get_neighborhood(const Matrix mat, int row, int col, int boundary_mode);
void apply_rule(Matrix out, const Matrix in, const Rule512* rule, int boundary_mode);
int simulate_with_depth(Matrix x_init, Matrix y_target, const Rule512* rule, int boundary_mode, int max_steps);
void compute_rule_number(const Rule512* rule, uint64_t* out);
void random_rule(Rule512* rule);

#ifdef __cplusplus
}
#endif

#endif
