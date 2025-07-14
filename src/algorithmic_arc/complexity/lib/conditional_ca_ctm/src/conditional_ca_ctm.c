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

#define RULE_BITS 512
#define RULE_UINT64_PARTS (RULE_BITS / 64)

volatile sig_atomic_t stop_requested = 0;

static void handle_sigint(int sig) {
    (void)sig;
    stop_requested = 1;
}

// Extracts 3×3 binary neighborhood as a 9-bit integer
static uint16_t get_neighborhood(const Matrix mat, int row, int col, int boundary_mode) {
    uint16_t code = 0;
    int idx = 0;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int r = row + dr;
            int c = col + dc;

            if (boundary_mode == 1) { // toroidal wrap
                r = (r + MATRIX_SIZE) % MATRIX_SIZE;
                c = (c + MATRIX_SIZE) % MATRIX_SIZE;
                code |= (mat[r][c] & 1) << idx++;
            } else { // zero padding
                if (r >= 0 && r < MATRIX_SIZE && c >= 0 && c < MATRIX_SIZE) {
                    code |= (mat[r][c] & 1) << idx;
                }
                idx++;
            }
        }
    }

    return code;
}

// Generates a random rule (512 bits / 64 bytes)
static void random_rule(Rule512* rule) {
    for (int i = 0; i < RULE_BYTES; ++i) {
        rule->table[i] = prng_next_byte();
    }
}

// Computes rule output for a given 9-bit neighborhood
static uint8_t get_rule_output(const Rule512* rule, uint16_t neighborhood) {
    int byte_index = neighborhood / 8;
    int bit_index  = neighborhood % 8;
    return (rule->table[byte_index] >> bit_index) & 1;
}

// Applies the rule to the full matrix
static void apply_rule(Matrix out, const Matrix in, const Rule512* rule, int boundary_mode) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            uint16_t nb = get_neighborhood(in, i, j, boundary_mode);
            out[i][j] = get_rule_output(rule, nb);
        }
    }
}

// Simulates until reaching y_target or looping. Returns depth or -1.
static int simulate_with_depth(Matrix x_init, Matrix y_target, const Rule512* rule, int boundary_mode, int max_steps) {
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
                return -1; // Cycle detected
            }
        }

        seen_hashes[seen_count++] = h;
        apply_rule(next, current, rule, boundary_mode);
        copy_matrix(current, next);
    }

    free(seen_hashes);
    return -1; // Target not reached
}

// Converts rule to 512-bit number as array of 8×uint64_t
static void compute_rule_number(const Rule512* rule, uint64_t* out) {
    for (int i = 0; i < RULE_UINT64_PARTS; ++i) {
        uint64_t part = 0;
        for (int j = 0; j < 8; ++j) {
            part = (part << 8) | rule->table[i * 8 + j];
        }
        out[i] = part;
    }
}

// Main entry point
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
    uint64_t*** match_rule_numbers,
    int** match_rule_depths,
    int* match_counts
) {
    stop_requested = 0;
    signal(SIGINT, handle_sigint);

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
        if (stop_requested) break;

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

        int count = match_counts[i];
        ms_out[i] = (double)count / num_rules;
        ctms_out[i] = (count > 0) ? -log2(ms_out[i]) : INFINITY;
    }

    free(rules);

    if (stop_requested) {
        fprintf(stderr, "Interrupted by user (SIGINT).\n");
    }
}

// Frees memory from the match tracking structures
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
