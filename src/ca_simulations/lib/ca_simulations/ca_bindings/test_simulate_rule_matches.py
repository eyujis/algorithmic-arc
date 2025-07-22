import numpy as np
from simulate_rule_matches_wrapper import simulate_rule_matches

def test_basic_pairs():
    xs = np.array([
        [[0,0,0,0],
         [0,1,1,0],
         [0,1,1,0],
         [0,0,0,0]],

        [[0,0,0,1],
         [0,1,0,0],
         [0,0,1,0],
         [1,0,0,0]]
    ], dtype=np.uint8)

    ys = np.array([
        [[0,0,0,0],
         [1,1,1,0],
         [1,1,1,0],
         [0,0,0,0]],

        [[0,0,0,0],
         [0,0,1,0],
         [0,1,0,0],
         [0,0,0,0]]
    ], dtype=np.uint8)

    print("Running CA simulation on 2 training pairs with 1,000,000 rules...")
    results = simulate_rule_matches(xs, ys, num_rules=1_000_000, seed=42, boundary_mode=1, max_steps=65536)

    for i, matches in enumerate(results):
        print(f"Pair {i}: {len(matches)} rules matched.")
        for j, (rule, depth) in enumerate(matches[:3]):
            print(f"  Rule {j}: rule={hex(rule)}, depth={depth}")

if __name__ == "__main__":
    test_basic_pairs()
