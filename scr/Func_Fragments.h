#ifndef FUNC_FRAGMENTS
#define FUNC_FRAGMENTS

int SUN_zero_fit(
    int N_STATION,
    double *target,
    double *can
);

int SUN_dark(
    int N_STATION,
    double *target
);


void Fragment_assign(
    struct df_rr_h *p_rrh,
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    int fragment
);

#endif