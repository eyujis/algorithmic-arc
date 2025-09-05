import numpy as np
from algorithmic_arc.dataset.dataset_builder import AlgorithmicARCDatasetBuilder
import pprint
from pathlib import Path

# === 1. Define transformation ===
def bit_inversion(matrix: np.ndarray) -> np.ndarray:
    return 1 - matrix

# === 2. Define 4×4 binary input matrices ===
x1 = np.array([
    [0, 0, 0, 0],
    [0, 0, 0, 0],
    [0, 0, 1, 1],
    [0, 0, 1, 1]
], dtype=np.uint8)

x2 = np.array([
    [0, 1, 1, 0],
    [0, 1, 1, 0],
    [0, 0, 0, 0],
    [0, 0, 0, 0]
], dtype=np.uint8)

x3 = np.array([
    [0, 0, 0, 0],
    [1, 1, 0, 0],
    [1, 1, 0, 0],
    [0, 0, 0, 0]
], dtype=np.uint8)

x4 = np.array([
    [0, 0, 0, 0],
    [0, 1, 1, 0],
    [0, 1, 1, 0],
    [0, 0, 0, 0]
], dtype=np.uint8)

x5 = np.array([
    [0, 0, 0, 0],
    [0, 0, 0, 0],
    [1, 1, 0, 0],
    [1, 1, 0, 0]
], dtype=np.uint8)


x_train_list = [x1, x2, x3, x4]
x_test_list = [x5]

# === 3. Resolve correct output path relative to this script ===
output_dir = Path(__file__).parent.parent / "data" / "easy"

# === 4. Instantiate the dataset builder ===
builder = AlgorithmicARCDatasetBuilder(
    output_dir=str(output_dir),
    transformation_name="bit_inversion",
    transformation_fn=bit_inversion,
    seed=42,
    boundary_mode=1
)

# === 5. Build and optionally save the task ===
task = builder.build_task(
    x_train_list=x_train_list,
    x_test_list=x_test_list,
    task_id=1,
    save=True
)

# === 6. Print task for visual confirmation ===
print("\n✅ Generated Task JSON:")
pprint.pprint(task)
