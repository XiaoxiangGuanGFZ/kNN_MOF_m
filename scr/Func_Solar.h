#ifndef FUNC_SOLAR
#define FUNC_SOLAR

extern FILE *p_SSIM;
extern int f_prep; 

void kNN_MOF_solar(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    double *Solar_MAX,
    int nrow_rr_d,
    int ndays_h
);

void similarity_SSIM_solar(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int *pool_cans,
    int n_can,
    int skip,
    double *SSIM
);

void Solar_MAX_class_derive(
    double **Solar_MAX,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int ndays_h
);

void Solar_MAX_class_preview(
    double *Solar_MAX,
    struct Para_global *p_gp
);

void Solar_MAX_class_filter(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    double *Solar_MAX,
    int *pool_cans,
    int n_can,
    int *n_can_out
);

/********* lumped MAX ********/

void Solar_MAX_lump_derive(
    double **Solar_MAX,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int ndays_h
);

void Solar_MAX_lump_preview(
    double *Solar_MAX,
    struct Para_global *p_gp
);

void Solar_MAX_lump_filter(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    double *Solar_MAX,
    int *pool_cans,
    int n_can,
    int *n_can_out
);


#endif