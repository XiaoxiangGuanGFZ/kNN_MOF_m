#ifndef FUNC_DISAGGREGATE
#define FUNC_DISAGGREGATE

extern FILE *p_SSIM;

void kNN_MOF_SSIM(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int nrow_rr_d,
    int ndays_h
);


// int Toggle_WD(
//     int N_STATION,
//     double *p_rr_d
// );

// int Toggle_CONTINUITY(
//     struct df_rr_h *p_rrh,
//     struct df_rr_d *p_rrd,
//     struct Para_global *p_gp,
//     int ndays_h,
//     int pool_cans[]
// );

void kNN_SSIM_sampling(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int pool_cans[],
    int n_can,
    int skip,
    int *index_fragment
);

double get_random();

int weight_cdf_sample(
    int size_pool,
    int pool_cans[],
    double *weights_cdf    
);


#endif