import numpy as np
import math

from ca_simulations import simulate_rule_matches
from ca_simulations import simulate_rule_outputs
from collections import defaultdict
from pybdm import BDM

class AlgorithmicAbductionInduction:
    bdm_1d = BDM(ndim=1)

    def __init__(self, num_rules=1_000_000, top_k=None, seed=42, boundary_mode=1, max_steps=65536, verbose=True):
        """
        Abduction–Induction method using cellular automata and algorithmic complexity.

        Parameters:
            num_rules (int): Number of CA rules to sample
            top_k (int): Number of top-ranked rules to use in induction (optional)
            seed (int): Seed for reproducible rule generation
            boundary_mode (int): 1 = toroidal, 0 = zero-padded
            max_steps (int): Max steps per CA simulation
            verbose (bool): Whether to print detailed logs
        """
        self.num_rules = num_rules
        self.top_k = top_k
        self.seed = seed
        self.boundary_mode = boundary_mode
        self.max_steps = max_steps
        self.verbose = verbose
        self.abducted_rules = None

    def _log(self, msg):
        if self.verbose:
            print(msg)

    # -------------------- Filtering Phase --------------------
    def filter_by_ctm_input(self, xs, ys, x_test, mode='absolute', threshold=2.0):
        bdm_2d = BDM(ndim=2, shape=(4, 4))
        key_test, c_test = next(bdm_2d.lookup([x_test]))
        self._log(f"[Filter] x_test complexity: {c_test:.2f} (key: {key_test})")

        filtered_xs, filtered_ys = [], []

        for x_i, y_i in zip(xs, ys):
            try:
                _, c_i = next(bdm_2d.lookup([x_i]))
            except Exception:
                continue

            if mode == 'absolute':
                if abs(c_i - c_test) <= threshold:
                    filtered_xs.append(x_i)
                    filtered_ys.append(y_i)
            elif mode == 'percent':
                if abs(c_i - c_test) / c_test <= threshold:
                    filtered_xs.append(x_i)
                    filtered_ys.append(y_i)
            else:
                raise ValueError("mode must be 'absolute' or 'percent'")

        self._log(f"[Filter] Retaining {len(filtered_xs)} / {len(xs)} training examples (mode={mode}, threshold={threshold})")
        if len(filtered_xs) == 0:
            self._log("[Filter] Warning: No training examples passed the complexity filter.")

        return np.array(filtered_xs, dtype=np.uint8), np.array(filtered_ys, dtype=np.uint8)

    # -------------------- Abduction Phase --------------------
    def abduct_rules(self, xs, ys):
        self._log(f"[Abduction] Searching for CA rules matching {len(xs)} training pairs...")

        results = simulate_rule_matches(
            xs,
            ys,
            num_rules=self.num_rules,
            seed=self.seed,
            boundary_mode=self.boundary_mode,
            max_steps=self.max_steps
        )

        rule_sets = []
        for matches in results:
            matched_rules = {rule_int for rule_int, _ in matches}
            rule_sets.append(matched_rules)

        common_rules = set.intersection(*rule_sets) if rule_sets else set()

        rule_to_matches = defaultdict(list)
        for i, matches in enumerate(results):
            for rule_int, depth in matches:
                if rule_int in common_rules:
                    rule_to_matches[rule_int].append((i, depth))

        self.abducted_rules = list(common_rules)
        self._log(f"[Abduction] Found {len(self.abducted_rules)} rules that match all training pairs.")

        return rule_to_matches

    # -------------------- Ranking Phase --------------------
    def rank_abducted_rules_by_bdm(self, rules):
        self._log(f"[Ranking] Ranking {len(rules)} rules by BDM complexity...")

        scored = []
        for rule_int in rules:
            rule_arr = self.rule_to_1d_array(rule_int)
            bdm_score = self.bdm_1d.bdm(rule_arr)
            scored.append((rule_int, bdm_score))

        ranked = sorted(scored, key=lambda t: t[1])

        for i, (rule, score) in enumerate(ranked[:5]):
            self._log(f"  Top {i+1}: Rule {rule} → BDM = {score:.2f}")

        return ranked

    @staticmethod
    def rule_to_1d_array(rule_int):
        bin_str = format(rule_int, '0512b')
        return np.array([int(b) for b in bin_str], dtype=int)

    # -------------------- Induction Phase --------------------
    def induce_outputs_from_rules(self, x_test, rules):
        self._log(f"[Induction] Applying {len(rules)} rules to x_test...")

        results = {}

        for rule_int in rules:
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

        for y_key, meta in results.items():
            meta['t_mean'] = float(np.mean(meta.pop('depths')))

        self._log(f"[Induction] Generated {len(results)} unique output candidates from rule applications.")
        return results

    def estimate_k_ctm(self, y_prime_data, total_rules, time_metric='t_min'):
        assert time_metric in ['t_min', 't_mean'], f"Invalid time_metric: {time_metric}"

        self._log(f"[Estimate] Estimating K(y|x_test) for {len(y_prime_data)} candidates using {total_rules} rules...")
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

        for i, (y_key, meta) in enumerate(list(scores.items())[:3]):
            self._log(f"  y'#{i+1}: k_ctm = {meta['k_ctm']:.2f}, m(y|x) = {meta['m_y_given_x']:.5f}, t_min = {meta['t_min']}")

        return scores

    def select_inductive_hypothesis(self, k_ctm_scores):
        self._log(f"[Selection] Selecting from {len(k_ctm_scores)} candidate outputs...")

        filtered = {
            y_key: data
            for y_key, data in k_ctm_scores.items()
            if data['t_min'] > 0 and np.isfinite(data['k_ctm'])
        }

        if not filtered:
            self._log("[Selection] No valid outputs (all t_min = 0 or k_ctm = ∞).")
            return None, []

        sorted_items = sorted(filtered.items(), key=lambda item: item[1]['k_ctm'])
        best_y_key, best_metadata = sorted_items[0]

        self._log(f"[Selection] Best output selected with k_ctm = {best_metadata['k_ctm']:.2f}")
        return best_metadata, sorted_items
    
    # -------------------- Pipeline --------------------
    def run(self, xs, ys, x_test):
        """
        Full abduction–induction pipeline (no filtering).

        Args:
            xs: List of input training matrices
            ys: List of output training matrices
            x_test: Test input matrix

        Returns:
            The best predicted output y' as a 4x4 matrix, or None if no valid output was found.
        """
        rule_matches = self.abduct_rules(xs, ys)

        if self.top_k is not None:
            ranked = self.rank_abducted_rules_by_bdm(list(rule_matches.keys()))
            top_rules = [r for r, _ in ranked[:self.top_k]]
        else:
            top_rules = list(rule_matches.keys())

        y_prime_data = self.induce_outputs_from_rules(x_test, top_rules)
        k_ctm_scores = self.estimate_k_ctm(y_prime_data, total_rules=self.num_rules)
        best_output, _ = self.select_inductive_hypothesis(k_ctm_scores)

        return best_output['matrix'] if best_output else None

    @staticmethod
    def _matrix_to_key(matrix):
        return tuple(matrix.flatten())

    @staticmethod
    def _key_to_matrix(key):
        return np.array(key, dtype=np.uint8).reshape((4, 4))

