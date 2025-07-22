import numpy as np
from simulate_rule_outputs_wrapper import simulate_rule_outputs

def test_handcrafted_rules():
    # Define a 4x4 binary matrix (same as x_flat in C)
    x = np.array([
        [0, 1, 1, 1],
        [0, 1, 1, 1],
        [0, 1, 1, 1],
        [0, 1, 1, 1]
    ], dtype=np.uint32)

    # Define 2 handcrafted rules (each 512 bits = 8 uint64)
    rules = np.array([
        [
            0x0123456789abcdef, 0xfedcba9876543210,
            0x0011223344556677, 0x8899aabbccddeeff,
            0xdeadbeefcafebabe, 0xfaceb00cdec0de01,
            0x1234567890abcdef, 0xf0e1d2c3b4a59687
        ],
        [
            0x0f0f0f0f0f0f0f0f, 0x1111111111111111,
            0x2222222222222222, 0x3333333333333333,
            0x4444444444444444, 0x5555555555555555,
            0x6666666666666666, 0x7777777777777777
        ]
    ], dtype=np.uint64)

    # Run simulation
    results = simulate_rule_outputs(x, rules, boundary_mode=1, max_steps=100)

    for r, (rule_int, outputs) in enumerate(results):
        print(f"Rule {r}:")
        # Print 512-bit rule ID in hex (reversed order for C-style printing)
        rule_hex = ''.join(f"{(rule_int >> (64 * i)) & 0xffffffffffffffff:016x}" for i in reversed(range(8)))
        print(f"  Rule ID: {rule_hex}")
        print(f"  Outputs: {len(outputs)}")
        for i, (matrix, depth) in enumerate(outputs[:3]):
            print(f"    Step {depth}:")
            print(matrix)
            print()

if __name__ == "__main__":
    test_handcrafted_rules()
