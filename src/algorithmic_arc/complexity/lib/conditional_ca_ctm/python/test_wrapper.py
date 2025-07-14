import numpy as np
from wrapper import compute_conditional_ca_ctm

# Define 2 input matrices (x) and their respective target outputs (y)
xs = np.array([
    [[0, 0, 0, 0],
     [0, 1, 1, 0],
     [0, 1, 1, 0],
     [0, 0, 0, 0]],

    [[0, 0, 1, 0],
     [0, 1, 0, 1],
     [1, 0, 1, 0],
     [0, 0, 1, 0]],
], dtype=np.uint8)

ys = np.array([
    [[0, 0, 0, 0],
     [0, 0, 1, 0],
     [0, 1, 0, 0],
     [0, 0, 0, 0]],

    [[0, 0, 0, 0],
     [1, 1, 1, 0],
     [1, 1, 1, 0],
     [0, 0, 0, 0]],
], dtype=np.uint8)

# Run the Conditional CA-CTM computation
results = compute_conditional_ca_ctm(xs, ys, num_rules=1000000, seed=123)

# Print results
for i, (ctm, m, matches) in enumerate(results):
    print(f"Pair {i}: CTM = {ctm:.5f}, m = {m:.5f}, matches = {len(matches)}")
    for rule, depth in matches[:3]:
        print(f"  Rule {rule} → depth {depth}")
