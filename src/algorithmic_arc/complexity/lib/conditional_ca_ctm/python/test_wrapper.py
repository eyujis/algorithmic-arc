import numpy as np
from wrapper import compute_conditional_ca_ctm

def print_matrix(name, mat):
    print(f"{name}:")
    print("[")
    for row in mat:
        print(" ", list(row))
    print("]")

# === Pair 1 ===
x1 = np.array([
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [0, 0, 1, 0],
    [0, 0, 0, 1]
], dtype=np.uint32)

y1 = np.array([
    [1, 1, 0, 0],
    [0, 1, 1, 0],
    [0, 0, 1, 1],
    [1, 0, 0, 1]
], dtype=np.uint32)

# === Pair 2 ===
x2 = np.array([
    [0, 1, 1, 0],
    [0, 1, 1, 0],
    [0, 1, 1, 0],
    [0, 1, 1, 0]
], dtype=np.uint32)

y2 = np.array([
    [1, 1, 0, 0],
    [1, 1, 0, 0],
    [1, 1, 0, 0],
    [1, 1, 0, 0]
], dtype=np.uint32)

# Stack input/output arrays
xs = np.stack([x1, x2])
ys = np.stack([y1, y2])


# Run the Conditional CA-CTM
results = compute_conditional_ca_ctm(xs, ys, num_rules=1_000_000, seed=123, boundary_mode=1)

# Show results
for i, (k, m) in enumerate(results):
    print(f"\nResult {i}: k(y|x) = {k:.4f}, m(y|x) = {m:.4f}")
