from cffi import FFI
import os

ffi = FFI()

# C function declaration (must match conditional_ca_ctm.h)
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
        double* ctm_out
    );
""")

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'libconditional_ca_ctm.so')
C = ffi.dlopen(lib_path)

# Python wrapper function
def compute_conditional_ca_ctm(xs, ys, num_rules=100000, seed=42, boundary_mode=1, max_steps=65536):
    import numpy as np

    num_pairs = len(xs)
    assert xs.shape == ys.shape
    assert xs.shape[1:] == (4, 4), "Each matrix must be 4x4"

    xs_flat = xs.reshape(num_pairs, 16).astype("uint32")
    ys_flat = ys.reshape(num_pairs, 16).astype("uint32")

    ms_out = np.zeros(num_pairs, dtype="float64")
    ctm_out = np.zeros(num_pairs, dtype="float64")

    C.run_ctm(
        ffi.cast("uint32_t*", xs_flat.ctypes.data),
        ffi.cast("uint32_t*", ys_flat.ctypes.data),
        num_pairs,
        num_rules,
        seed,
        boundary_mode,
        max_steps,
        ffi.cast("double*", ms_out.ctypes.data),
        ffi.cast("double*", ctm_out.ctypes.data),
    )

    return list(zip(ctm_out, ms_out))
