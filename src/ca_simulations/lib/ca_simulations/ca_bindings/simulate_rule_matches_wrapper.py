import os
import platform
import numpy as np
from cffi import FFI

ffi = FFI()

# C function declaration (updated: no ms_out, ctms_out)
ffi.cdef("""
    void simulate_rule_matches(
        uint32_t* xs_flat,
        uint32_t* ys_flat,
        int num_pairs,
        int num_rules,
        unsigned int seed,
        int boundary_mode,
        int max_steps,
        uint64_t*** match_rule_numbers,
        int** match_rule_depths,
        int* match_counts
    );

    void free_matches(
        int num_pairs,
        int* match_counts,
        int** match_rule_depths,
        uint64_t*** match_rule_numbers
    );
""")

# Determine correct shared library extension
ext = 'dylib' if platform.system() == 'Darwin' else 'so'
lib_name = f'libsimulate_rule_matches.{ext}'  # <- Updated library name
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', lib_name))

# Load shared library
C = ffi.dlopen(lib_path)

def simulate_rule_matches(xs, ys, num_rules=1_000_000, seed=42, boundary_mode=1, max_steps=65536):
    num_pairs = len(xs)
    assert xs.shape == ys.shape
    assert xs.shape[1:] == (4, 4), "Each matrix must be 4×4"

    xs_flat = xs.reshape(num_pairs, 16).astype("uint32")
    ys_flat = ys.reshape(num_pairs, 16).astype("uint32")

    match_counts = ffi.new("int[]", num_pairs)
    match_rule_depths = ffi.new("int*[]", num_pairs)
    match_rule_numbers = ffi.new("uint64_t**[]", num_pairs)

    C.simulate_rule_matches(
        ffi.cast("uint32_t*", xs_flat.ctypes.data),
        ffi.cast("uint32_t*", ys_flat.ctypes.data),
        num_pairs,
        num_rules,
        seed,
        boundary_mode,
        max_steps,
        match_rule_numbers,
        match_rule_depths,
        match_counts
    )

    # Decode results
    results = []
    for i in range(num_pairs):
        count = match_counts[i]
        matches = []
        for j in range(count):
            rule_ptr = match_rule_numbers[i][j]
            rule_int = 0
            for k in range(8):
                rule_int |= int(rule_ptr[k]) << (64 * k)
            depth = match_rule_depths[i][j]
            matches.append((rule_int, depth))
        results.append(matches)

    # Free C-side memory
    C.free_matches(num_pairs, match_counts, match_rule_depths, match_rule_numbers)

    return results
