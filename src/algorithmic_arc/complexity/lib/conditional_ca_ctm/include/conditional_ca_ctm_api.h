#ifndef CONDITIONAL_CA_CTM_API_H
#define CONDITIONAL_CA_CTM_API_H

#ifdef __cplusplus
extern "C" {
#endif

void run_ctm(
    uint32_t* xs_flat,
    uint32_t* ys_flat,
    int num_pairs,
    int num_rules,
    unsigned int seed,
    int boundary_mode,
    int max_steps,
    double* ms_out,
    double* ks_out
);

#ifdef __cplusplus
}
#endif

#endif
