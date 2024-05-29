#ifndef FUNC_PREPRO
#define FUNC_PREPRO

void Normalize(
    struct Para_global *p_gp,
    struct df_rr_d *p_rr_d,
    struct df_rr_h *p_rr_h,
    int nrow_d,
    int nrow_h);

void Standardize(
    struct Para_global *p_gp,
    struct df_rr_d *p_rr_d,
    struct df_rr_h *p_rr_h,
    int nrow_d,
    int nrow_h);
    
#endif