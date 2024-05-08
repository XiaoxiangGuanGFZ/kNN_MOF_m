#ifndef FUNC_COVAR
#define FUNC_COVAR

void kNN_MOF_cov(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh_cov,
    struct df_rr_d *p_rrd_cov,
    struct Para_global *p_gp,
    int nrow_rr_d,
    int ndays_h
);

void similarity_SSIM(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int *pool_cans,
    int n_can,
    int skip,
    double **SSIM
);


#endif