#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ca_dynamics.h"
#include "matrix_utils.h"

#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 65536
#endif

#define RULE_BITS 512
#define RULE_UINT64_PARTS (RULE_BITS / 64)

uint16_t get_neighborhood(const Matrix mat, int row, int col, int boundary_mode) {
    uint16_t code = 0;
    int idx = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int r = row + dr;
            int c = col + dc;

            if (boundary_mode == 1) { // toroidal
                r = (r + MATRIX_SIZE) % MATRIX_SIZE;
                c = (c + MATRIX_SIZE) % MATRIX_SIZE;
                code |= (mat[r][c] & 1) << idx++;
            } else {
                if (r >= 0 && r < MATRIX_SIZE && c >= 0 && c < MATRIX_SIZE) {
                    code |= (mat[r][c] & 1) << idx;
                }
                idx++;
            }
        }
    }
    return code;
}

void apply_rule(Matrix out, const Matrix in, const Rule512* rule, int boundary_mode) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            uint16_t nb = get_neighborhood(in, i, j, boundary_mode);
            int byte_index = nb / 8;
            int bit_index = nb % 8;
            out[i][j] = (rule->table[byte_index] >> bit_index) & 1;
        }
    }
}

int simulate_with_depth(Matrix x_init, Matrix y_target, const Rule512* rule, int boundary_mode, int max_steps) {
    if (max_steps <= 0) max_steps = DEFAULT_MAX_STEPS;

    Matrix current, next;
    uint64_t* seen_hashes = malloc(sizeof(uint64_t) * max_steps);
    if (!seen_hashes) {
        fprintf(stderr, "Memory allocation failed for hash tracking.\n");
        exit(EXIT_FAILURE);
    }

    int seen_count = 0;
    copy_matrix(current, x_init);

    for (int t = 0; t < max_steps; ++t) {
        if (matrix_equals(current, y_target)) {
            free(seen_hashes);
            return t;
        }

        uint64_t h = matrix_hash(current);
        for (int i = 0; i < seen_count; ++i) {
            if (seen_hashes[i] == h) {
                free(seen_hashes);
                return -1;
            }
        }

        seen_hashes[seen_count++] = h;
        apply_rule(next, current, rule, boundary_mode);
        copy_matrix(current, next);
    }

    free(seen_hashes);
    return -1;
}

void compute_rule_number(const Rule512* rule, uint64_t* out) {
    for (int i = 0; i < RULE_UINT64_PARTS; ++i) {
        uint64_t part = 0;
        for (int j = 0; j < 8; ++j) {
            part = (part << 8) | rule->table[i * 8 + j];
        }
        out[i] = part;
    }
}

// Optional: For rule sampling from a PRNG
#include <prng/prng.h>

void random_rule(Rule512* rule) {
    for (int i = 0; i < RULE_BYTES; ++i) {
        rule->table[i] = prng_next_byte();
    }
}
