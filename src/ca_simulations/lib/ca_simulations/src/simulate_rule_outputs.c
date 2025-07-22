#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "interrupt_flag.h"
#include "matrix_utils.h"
#include "simulate_rule_outputs.h"
#include "ca_dynamics.h"

#ifndef DEFAULT_MAX_STEPS
#define DEFAULT_MAX_STEPS 65536
#endif

void simulate_rule_outputs(
    uint32_t* x_flat,
    uint64_t* rules_flat,
    int num_rules,
    int boundary_mode,
    int max_steps,
    OutputMap** output_maps_out
) {
    init_interrupt_flag();  // Set up shared interrupt signal handler

    if (max_steps <= 0) max_steps = DEFAULT_MAX_STEPS;

    Rule512* rules = calloc(num_rules, sizeof(Rule512));
    if (!rules) {
        fprintf(stderr, "Memory allocation failed for rules.\n");
        exit(EXIT_FAILURE);
    }

    // Decode rules from flattened uint64_t[8] format into 64-byte tables
    for (int r = 0; r < num_rules; ++r) {
        for (int i = 0; i < 8; ++i) {
            uint64_t part = rules_flat[r * 8 + i];
            for (int j = 0; j < 8; ++j) {
                rules[r].table[i * 8 + (7 - j)] = (part >> (8 * j)) & 0xFF;
            }
        }
    }

    OutputMap* output_maps = calloc(num_rules, sizeof(OutputMap));
    if (!output_maps) {
        fprintf(stderr, "Memory allocation failed for output maps.\n");
        exit(EXIT_FAILURE);
    }

    Matrix current, next;
    Matrix x;
    flat_to_matrix(x, x_flat);

    for (int r = 0; r < num_rules; ++r) {
        if (is_interrupted()) break;

        copy_matrix(current, x);

        OutputMap* map = &output_maps[r];
        memcpy(map->rule_number, &rules_flat[r * 8], sizeof(uint64_t) * 8);

        map->outputs = malloc(max_steps * sizeof(Matrix));
        map->depths = malloc(max_steps * sizeof(int));
        map->num_outputs = 0;

        if (!map->outputs || !map->depths) {
            fprintf(stderr, "Memory allocation failed for output tracking.\n");
            exit(EXIT_FAILURE);
        }

        uint64_t* seen_hashes = malloc(max_steps * sizeof(uint64_t));
        if (!seen_hashes) {
            fprintf(stderr, "Memory allocation failed for hash tracking.\n");
            exit(EXIT_FAILURE);
        }

        int seen_count = 0;

        for (int t = 0; t < max_steps; ++t) {
            if (is_interrupted()) break;

            uint64_t h = matrix_hash(current);
            int already_seen = 0;
            for (int i = 0; i < seen_count; ++i) {
                if (seen_hashes[i] == h) {
                    already_seen = 1;
                    break;
                }
            }
            if (already_seen) break;

            seen_hashes[seen_count++] = h;

            copy_matrix(map->outputs[map->num_outputs], current);
            map->depths[map->num_outputs] = t;
            map->num_outputs++;

            apply_rule(next, current, &rules[r], boundary_mode);
            copy_matrix(current, next);
        }

        free(seen_hashes);
    }

    free(rules);
    *output_maps_out = output_maps;

    if (is_interrupted()) {
        fprintf(stderr, "Interrupted by user (SIGINT).\n");
    }
}

void free_output_maps(int num_rules, OutputMap* output_maps) {
    if (!output_maps) return;

    for (int r = 0; r < num_rules; ++r) {
        free(output_maps[r].outputs);
        free(output_maps[r].depths);
    }
    free(output_maps);
}
