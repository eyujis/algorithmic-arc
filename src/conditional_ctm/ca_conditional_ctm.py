import math
from ca_simulations import simulate_rule_matches

class CAConditionalCTM:
    def __init__(self, num_rules=1_000_000, seed=42, boundary_mode=1, max_steps=65536):
        self.num_rules = num_rules
        self.seed = seed
        self.boundary_mode = boundary_mode
        self.max_steps = max_steps

    def compute(self, xs, ys):
        """
        Parameters:
            xs (np.ndarray): shape (N, 4, 4), input X matrices
            ys (np.ndarray): shape (N, 4, 4), target Y matrices

        Returns:
            List[Dict]: Each dict contains 'match_count', 'm', 'ctm', 'min_depth'
        """
        match_data = simulate_rule_matches(
            xs=xs,
            ys=ys,
            num_rules=self.num_rules,
            seed=self.seed,
            boundary_mode=self.boundary_mode,
            max_steps=self.max_steps
        )

        results = []
        for matches in match_data:
            count = len(matches)
            m = count / self.num_rules
            ctm = -math.log2(m) if m > 0 else float("inf")
            min_depth = min((depth for _, depth in matches), default=None)

            results.append({
                "match_count": count,
                "m": m,
                "ctm": ctm,
                "min_depth": min_depth,
                "matches": matches
            })

        return results
