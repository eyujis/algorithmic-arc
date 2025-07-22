import numpy as np
import math

from ca_simulations import simulate_rule_matches
from ca_simulations import simulate_rule_outputs
from collections import defaultdict
from pybdm import BDM

class AlgorithmicAbductionInduction:
    bdm_1d = BDM(ndim=1)

    def __init__(self, num_rules=1_000_000, top_k=None, seed=42, boundary_mode=1, max_steps=65536):
        """
        Parameters:
            num_rules (int): Number of CA rules to sample
            top_k (int): How many rules to keep after ranking (optional)
            rule (int): Seed for reproducible rule generation
            boundary_mode (int): 1 = toroidal, 0 = zero-padded
            max_steps (int): Max steps per CA simulation
        """
        self.num_rules = num_rules
        self.top_k = top_k
        self.seed = seed
        self.boundary_mode = boundary_mode
        self.max_steps = max_steps
        self.abducted_rules = None  # Will store set or list of good rules

    # -------------------- Abduction Phase --------------------

    def abduct_rules(self, xs, ys):
        """
        For each (x, y), find CA rules that generate y from x.
        Stores matched rules in self.abducted_rules.
        Returns: dict of rule → list of (pair_index, depth), but only for rules matching all pairs.
        """
        print("[Abduction] Simulating rule matches on training pairs...")
        results = simulate_rule_matches(
            xs,
            ys,
            num_rules=self.num_rules,
            seed=self.seed,
            boundary_mode=self.boundary_mode,
            max_steps=self.max_steps
        )

        # First collect the set of matching rules per pair
        rule_sets = []
        for matches in results:
            matched_rules = {rule_int for rule_int, _ in matches}
            rule_sets.append(matched_rules)

        # Find rules that match all (x, y) pairs
        common_rules = set.intersection(*rule_sets) if rule_sets else set()

        # Filter to only include common rules in the output mapping
        rule_to_matches = defaultdict(list)
        for i, matches in enumerate(results):
            for rule_int, depth in matches:
                if rule_int in common_rules:
                    rule_to_matches[rule_int].append((i, depth))

        self.abducted_rules = list(common_rules)

        print(f"[Abduction] Found {len(self.abducted_rules)} rules that match all training pairs.")
        return rule_to_matches

    # -------------------- Ranking Phase --------------------

    def rank_abducted_rules_by_bdm(self, rules):
        """
        Rank 512-bit rules using 1D BDM complexity.
        Input:
            rules (List[int]) — list of rule integers
        Output:
            List[(rule_int, bdm_score)] sorted by ascending BDM
        """
        print("[Ranking] Computing 1D BDM for rules...")

        scored = []
        for rule_int in rules:
            rule_arr = self.rule_to_1d_array(rule_int)
            bdm_score = self.bdm_1d.bdm(rule_arr)
            scored.append((rule_int, bdm_score))

        ranked = sorted(scored, key=lambda t: t[1])
        print(f"[Ranking] Done. Ranked {len(ranked)} rules by simplicity.")
        return ranked
    
    @staticmethod
    def rule_to_1d_array(rule_int):
        """Convert a 512-bit int into a 1D binary array of 0s and 1s."""
        bin_str = format(rule_int, '0512b')
        return np.array([int(b) for b in bin_str], dtype=int)

    # -------------------- Induction Phase --------------------

    def induce_outputs_from_rules(self, x_test, rules):
        """
        Apply each rule to x_test and collect reachable outputs.
        Output:
            Dict[y'_key] = {
                'rules': [...],
                't_min': int,
                't_mean': float,
                'matrix': np.array
            }
        """
        results = {}

        for rule_int in rules:
            # Convert 512-bit rule integer to 8×uint64 parts
            rule_parts = [(rule_int >> (64 * i)) & ((1 << 64) - 1) for i in range(8)]
            rules_flat = np.array(rule_parts, dtype=np.uint64).reshape(1, 8)

            simulation = simulate_rule_outputs(
                x=x_test,
                rules=rules_flat,
                boundary_mode=self.boundary_mode,
                max_steps=self.max_steps
            )

            rule_number, outputs = simulation[0]

            for matrix, depth in outputs:
                y_key = self._matrix_to_key(matrix)
                if y_key not in results:
                    results[y_key] = {
                        'rules': [rule_number],
                        'depths': [depth],
                        't_min': depth,
                        'matrix': matrix
                    }
                else:
                    results[y_key]['rules'].append(rule_number)
                    results[y_key]['depths'].append(depth)
                    results[y_key]['t_min'] = min(results[y_key]['t_min'], depth)

        # Compute t_mean for each entry
        for y_key, meta in results.items():
            meta['t_mean'] = float(np.mean(meta.pop('depths')))

        return results


    def estimate_k_ctm(self, y_prime_data, total_rules, time_metric='t_min'):
        """
        Estimate conditional complexity and metadata for each y' using:
            K(y'|x_test) = -log2(freq) + log2(time)
        where time is either t_min (default) or t_mean.

        Args:
            y_prime_data (dict): Dict[y_key] = {
                'rules': [...], 't_min': int, 't_mean': float, 'matrix': np.array
            }
            total_rules (int): Total number of rules evaluated (for computing frequency)
            time_metric (str): One of ['t_min', 't_mean'], default = 't_min'

        Returns:
            Dict[y_key] = {
                'k_ctm': float,
                'num_rules': int,
                'm_y_given_x': float,
                't_min': int,
                't_mean': float,
                'matrix': np.array
            }
        """
        assert time_metric in ['t_min', 't_mean'], f"Invalid time_metric: {time_metric}"

        scores = {}

        for y_key, data in y_prime_data.items():
            num_rules = len(data['rules'])
            freq = num_rules / total_rules
            t_min = data['t_min']
            t_mean = data['t_mean']
            time_val = t_min if time_metric == 't_min' else t_mean

            if freq <= 0 or time_val <= 0:
                scores[y_key] = {
                    'k_ctm': float('inf'),
                    'num_rules': num_rules,
                    'm_y_given_x': 0.0,
                    't_min': t_min,
                    't_mean': t_mean,
                    'matrix': data['matrix']
                }
                continue

            k_ctm = -math.log2(freq) + math.log2(time_val)

            scores[y_key] = {
                'k_ctm': k_ctm,
                'num_rules': num_rules,
                'm_y_given_x': freq,
                't_min': t_min,
                't_mean': t_mean,
                'matrix': data['matrix']
            }

        return scores


    def select_inductive_hypothesis(self, k_ctm_scores):
        """
        Select the y' with the lowest estimated conditional complexity,
        excluding outputs that appeared at step 0 (i.e., equal to x_test).

        Args:
            k_ctm_scores (dict): Dict[y_key] = {
                'k_ctm': float,
                'num_rules': int,
                'prob': float,
                't_min': int,
                'matrix': np.ndarray
            }

        Returns:
            Tuple:
                - selected_hypothesis (dict)
                - sorted_candidates (List[Tuple[y_key, metadata_dict]])
        """
        # Filter out outputs that appeared at step 0
        filtered = {
            y_key: data
            for y_key, data in k_ctm_scores.items()
            if data['t_min'] > 0 and np.isfinite(data['k_ctm'])
        }

        if not filtered:
            return None, []

        # Sort by k_ctm
        sorted_items = sorted(filtered.items(), key=lambda item: item[1]['k_ctm'])
        best_y_key, best_metadata = sorted_items[0]

        return best_metadata, sorted_items

    
    @staticmethod
    def _matrix_to_key(matrix):
        """Convert 4×4 binary matrix to hashable, reversible tuple key."""
        return tuple(matrix.flatten())

    @staticmethod
    def _key_to_matrix(key):
        """Convert hashable key back to 4×4 matrix."""
        return np.array(key, dtype=np.uint8).reshape((4, 4))

    # -------------------- Pipeline --------------------

    def run(self, xs, ys, x_test):
        """
        Full abduction + induction pipeline.
        Returns the best predicted output y' for x_test.
        """
        raise NotImplementedError

