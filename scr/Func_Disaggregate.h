#ifndef FUNC_DISAGGREGATE
#define FUNC_DISAGGREGATE

extern FILE *p_SSIM;
extern int f_prep; 

void kNN_MOF_SSIM(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int nrow_rr_d,
    int ndays_h
);


void kNN_SSIM_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int pool_cans[],
    int order,
    int n_can,
    int skip,
    int run,
    int *index_fragment
);


void Rhu_MAX_class_filter(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int *pool_cans,
    int n_can,
    int *n_can_out
);


#endif