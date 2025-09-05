import os
import json
import numpy as np
from typing import List, Callable, Optional

class AlgorithmicARCDatasetBuilder:
    """
    Builder class to generate Algorithmic-ARC tasks from specified training and test inputs.

    Responsibilities:
    - Receive input matrices (x_i for train, x_j for test)
    - Apply a given transformation function to generate y_i and y_j
    - Build a task dictionary in ARC-like format (with minimal metadata)
    - Optionally save the task to a JSON file
    """

    def __init__(self,
                 output_dir: str,
                 transformation_name: str,
                 transformation_fn: Callable[[np.ndarray], np.ndarray],
                 seed: Optional[int] = None,
                 boundary_mode: int = 1):
        """
        Parameters:
        - output_dir: Path to save the generated task file
        - transformation_name: Human-readable name of the transformation (used in filename)
        - transformation_fn: Function to transform an input x into output y
        - seed: RNG seed for reproducibility
        - boundary_mode: 1 = toroidal, 0 = zero-padded (recorded but not used here)
        """
        self.output_dir = output_dir
        self.transformation_name = transformation_name
        self.transformation_fn = transformation_fn
        self.seed = seed if seed is not None else None
        self.boundary_mode = boundary_mode
        os.makedirs(self.output_dir, exist_ok=True)

    def apply_transformation(self, x_list: List[np.ndarray]) -> List[np.ndarray]:
        """
        Apply the transformation function to a list of input matrices.
        """
        return [self.transformation_fn(x) for x in x_list]

    def build_task(self,
                   x_train_list: List[np.ndarray],
                   x_test_list: List[np.ndarray],
                   task_id: Optional[int] = None,
                   save: bool = True) -> dict:
        """
        Build the task JSON (train/test pairs + metadata).
        Optionally save to disk.

        Returns:
            The task dictionary (suitable for saving or further processing)
        """
        y_train_list = self.apply_transformation(x_train_list)
        y_test_list = self.apply_transformation(x_test_list)

        task_data = {
            "train": [{"input": x.tolist(), "output": y.tolist()}
                      for x, y in zip(x_train_list, y_train_list)],
            "test": [{"input": x.tolist(), "output": y.tolist()}
                     for x, y in zip(x_test_list, y_test_list)],
            "metadata": {
                "transformation": self.transformation_name,
                "seed": self.seed,
                "boundary_mode": self.boundary_mode
            }
        }

        if save and task_id is not None:
            task_name = f"task_{task_id:04d}_{self.transformation_name}_{self.seed}.json"
            self.save_task(task_data, task_name)

        return task_data

    def save_task(self, task_data: dict, filename: str):
        """
        Save the task dictionary as a JSON file in the output directory.
        """
        path = os.path.join(self.output_dir, filename)
        with open(path, "w") as f:
            json.dump(task_data, f, indent=2)

