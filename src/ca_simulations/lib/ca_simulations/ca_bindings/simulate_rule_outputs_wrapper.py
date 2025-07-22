import os
import platform
import numpy as np
from cffi import FFI

ffi = FFI()

# === C Declarations ===
ffi.cdef("""
    typedef struct {
        uint64_t rule_number[8];
        uint8_t (*outputs)[4][4];
        int* depths;
        int num_outputs;
    } OutputMap;

    void simulate_rule_outputs(
        uint32_t* x_flat,
        uint64_t* rules_flat,
        int num_rules,
        int boundary_mode,
        int max_steps,
        OutputMap** output_maps_out
    );

    void free_output_maps(
        int num_rules,
        OutputMap* output_maps
    );
""")

# === Load shared library ===
ext = 'dylib' if platform.system() == 'Darwin' else 'so'
lib_name = f'libsimulate_rule_matches.{ext}'
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', lib_name))
C = ffi.dlopen(lib_path)

# === Main wrapper ===
def simulate_rule_outputs(x, rules, boundary_mode=1, max_steps=65536):
    assert x.shape == (4, 4), "Input matrix must be 4×4"
    assert isinstance(rules, (list, np.ndarray)), "Rules must be list or numpy array"
    rules = np.asarray(rules, dtype=np.uint64)
    assert rules.ndim == 2 and rules.shape[1] == 8, "Each rule must be 8×uint64 (512-bit)"

    num_rules = rules.shape[0]
    x_flat = x.astype("uint32").flatten()
    rules_flat = rules.flatten()

    output_maps_ptr = ffi.new("OutputMap**")

    C.simulate_rule_outputs(
        ffi.cast("uint32_t*", x_flat.ctypes.data),
        ffi.cast("uint64_t*", rules_flat.ctypes.data),
        num_rules,
        boundary_mode,
        max_steps,
        output_maps_ptr
    )

    output_maps = output_maps_ptr[0]
    results = []

    for r in range(num_rules):
        rule_struct = output_maps[r]
        rule_number = sum(int(rule_struct.rule_number[k]) << (64 * k) for k in range(8))

        outputs = []
        for i in range(rule_struct.num_outputs):
            matrix = np.zeros((4, 4), dtype=np.uint8)
            for row in range(4):
                for col in range(4):
                    matrix[row, col] = rule_struct.outputs[i][row][col]
            outputs.append((matrix, int(rule_struct.depths[i])))

        results.append((rule_number, outputs))

    C.free_output_maps(num_rules, output_maps)
    return results
