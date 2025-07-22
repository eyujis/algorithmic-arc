#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "simulate_rule_outputs.h"
#include "matrix_utils.h"
#include "ca_dynamics.h"

int main() {
    // Define a 4x4 input matrix (flattened)
    uint32_t x_flat[16] = {
        0, 1, 1, 1,
        0, 1, 1, 1,
        0, 1, 1, 1,
        0, 1, 1, 1
    };

    // Define a small number of handcrafted rules for testing (2 in this example)
    // Each rule is represented by 8 uint64_t chunks (512 bits total)
    uint64_t rules_flat[2 * 8] = {
        0x0123456789abcdef, 0xfedcba9876543210,
        0x0011223344556677, 0x8899aabbccddeeff,
        0xdeadbeefcafebabe, 0xfaceb00cdec0de01,
        0x1234567890abcdef, 0xf0e1d2c3b4a59687,

        0x0f0f0f0f0f0f0f0f, 0x1111111111111111,
        0x2222222222222222, 0x3333333333333333,
        0x4444444444444444, 0x5555555555555555,
        0x6666666666666666, 0x7777777777777777
    };

    int num_rules = 2;
    int boundary_mode = 1;  // Toroidal
    int max_steps = 100;

    OutputMap* output_maps = NULL;

    simulate_rule_outputs(
        x_flat,
        rules_flat,
        num_rules,
        boundary_mode,
        max_steps,
        &output_maps
    );

    // Print summary for each rule
    for (int r = 0; r < num_rules; ++r) {
        printf("Rule %d:\n", r);
        printf("  Rule ID: ");
        for (int i = 7; i >= 0; --i) {
            printf("%016llx", (unsigned long long)output_maps[r].rule_number[i]);
        }
        printf("\n  Outputs: %d\n", output_maps[r].num_outputs);

        for (int i = 0; i < output_maps[r].num_outputs && i < 3; ++i) {
            printf("    Step %d:\n", output_maps[r].depths[i]);
            print_matrix(output_maps[r].outputs[i]);
            printf("\n");
        }
    }

    free_output_maps(num_rules, output_maps);
    return 0;
}
