from cffi import FFI
import os
import numpy as np

ffi = FFI()

# Match the updated C header
ffi.cdef("""
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
    );
""")

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'libconditional_ca_ctm.so')
C = ffi.dlopen(lib_path)

def compute_conditional_ca_ctm(xs, ys, num_rules=100000, seed=42, boundary_mode=1, max_steps=65536):
    num_pairs = len(xs)
    assert xs.shape == ys.shape
    assert xs.shape[1:] == (4, 4), "Each matrix must be 4x4"

    xs_flat = xs.reshape(num_pairs, 16).astype("uint32")
    ys_flat = ys.reshape(num_pairs, 16).astype("uint32")

    ms_out = np.zeros(num_pairs, dtype="float64")
    ctms_out = np.zeros(num_pairs, dtype="float64")

    # Allocate match_counts (how many rules matched for each pair)
    match_counts = ffi.new("int[]", num_pairs)

    # Allocate arrays of pointers for each pair
    match_rule_indices = ffi.new("int*[]", num_pairs)
    match_rule_depths = ffi.new("int*[]", num_pairs)

    C.run_ctm(
        ffi.cast("uint32_t*", xs_flat.ctypes.data),
        ffi.cast("uint32_t*", ys_flat.ctypes.data),
        num_pairs,
        num_rules,
        seed,
        boundary_mode,
        max_steps,
        ffi.cast("double*", ms_out.ctypes.data),
        ffi.cast("double*", ctms_out.ctypes.data),
        match_rule_indices,
        match_rule_depths,
        match_counts
    )

    results = []
    for i in range(num_pairs):
        count = match_counts[i]
        matches = []
        for j in range(count):
            rule_idx = match_rule_indices[i][j]
            depth = match_rule_depths[i][j]
            matches.append((rule_idx, depth))
        results.append((ctms_out[i], ms_out[i], matches))

    return results
