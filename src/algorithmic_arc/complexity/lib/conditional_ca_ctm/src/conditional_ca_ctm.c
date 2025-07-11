#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <signal.h>

#include <prng/prng.h>
#include <prng/splitmix64.h>
#include <prng/xoshiro256plusplus.h>
#include <matrix_utils.h>
#include <conditional_ca_ctm.h>

#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 65536
#endif

volatile sig_atomic_t stop_requested = 0;

void handle_sigint(int sig) {
    stop_requested = 1;
}

uint8_t get_neighborhood(const Matrix mat, int row, int col, int boundary_mode) {
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
                if (r < 0 || r >= MATRIX_SIZE || c < 0 || c >= MATRIX_SIZE) // zero padded
                    code |= 0 << idx++;
                else
                    code |= (mat[r][c] & 1) << idx++;
            }
        }
    }
    return code;
}

void random_rule(Rule512* rule) {
    for (int i = 0; i < RULE_BYTES; ++i) {
        rule->table[i] = prng_next_byte();
    }
}

uint8_t get_rule_output(const Rule512* rule, uint16_t neighborhood) {
    int byte_index = neighborhood / 8;
    int bit_index = neighborhood % 8;
    return (rule->table[byte_index] >> bit_index) & 1;
}

void apply_rule(Matrix out, const Matrix in, const Rule512* rule, int boundary_mode) {
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            uint8_t neighborhood = get_neighborhood(in, i, j, boundary_mode);
            out[i][j] = get_rule_output(rule, neighborhood);
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
        if (stop_requested) {
            free(seen_hashes);
            return -1;
        }

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
    int** match_rule_indices,
    int** match_rule_depths,
    int* match_counts
) {
    stop_requested = 0;
    signal(SIGINT, handle_sigint);

    if (max_steps <= 0) max_steps = DEFAULT_MAX_STEPS;

    prng_seed(seed);

    Rule512* rules = malloc(sizeof(Rule512) * num_rules);
    if (!rules) {
        fprintf(stderr, "Memory allocation failed for rules.\n");
        exit(EXIT_FAILURE);
    }

    for (int r = 0; r < num_rules; ++r) {
        random_rule(&rules[r]);
    }

    Matrix x, y;

    for (int i = 0; i < num_pairs; ++i) {
        if (stop_requested) break;

        flat_to_matrix(x, &xs_flat[i * 16]);
        flat_to_matrix(y, &ys_flat[i * 16]);

        match_counts[i] = 0;

        // Temporary storage (over-allocate up to num_rules)
        match_rule_indices[i] = malloc(sizeof(int) * num_rules);
        match_rule_depths[i]  = malloc(sizeof(int) * num_rules);

        if (!match_rule_indices[i] || !match_rule_depths[i]) {
            fprintf(stderr, "Memory allocation failed for match results.\n");
            exit(EXIT_FAILURE);
        }

        for (int r = 0; r < num_rules; ++r) {
            int depth = simulate_with_depth(x, y, &rules[r], boundary_mode, max_steps);
            if (depth >= 0) {
                int idx = match_counts[i];
                match_rule_indices[i][idx] = r;
                match_rule_depths[i][idx] = depth;
                match_counts[i]++;
            }
        }

        int count = match_counts[i];
        ms_out[i] = (double)count / num_rules;
        ctms_out[i] = (count > 0) ? -log2(ms_out[i]) : INFINITY;
    }

    free(rules);

    if (stop_requested) {
        fprintf(stderr, "Interrupted by user (Ctrl+C).\n");
    }
}
