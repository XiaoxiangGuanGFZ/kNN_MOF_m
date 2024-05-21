
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "def_struct.h"
#include "Func_kNN.h"
#include "Func_Solar.h"
#include "Func_Fragments.h"
#include "Func_dataIO.h"
#include "Func_SSIM.h"


void kNN_MOF_solar(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    double *Solar_MAX,
    int nrow_rr_d,
    int ndays_h
) {
    /*******************
     * Description:
     *  algorithm: k-nearest neighbouring sampling, method-of-fragments, based on seasonality
     *  scale: daily2hourly, multiple stations simultaneously
     * Parameters:
     *  nrow_rr_d: the number of rows in daily data file
     *  ndays_h: the number of observations of hourly data
     * *****************/
    int i, j, h;
    int class_t, class_c;

    /*********
     * order: 
     * - 1: sort the similarity metric in decreasing order: SSIM
     * - 0: sort the similarity metric in increasing order: Distance
     * ******/
    int order = 1;
    struct df_rr_h df_rr_h_out; // this is a struct variable, not a struct array;
    
    /************
     * CONTINUITY and skip
     * - p_gp->CONTINUITY: 1, skip = 0;
     * - p_gp->CONTINUITY: 3, skip = 1;
     * - p_gp->CONTINUITY: 5, skip = 2;
     * *************/
    int skip = 0;
    skip = (int)((p_gp->CONTINUITY - 1) / 2);
    int pool_cans[MAXrow]; // the index of the candidates (a pool); the size is sufficient
    int n_can;             // the number of candidates after all conditioning
    int fragment;          // the index of df_rr_h structure with the final chosed fragments

    FILE *p_FP_OUT;
    if ((p_FP_OUT = fopen(p_gp->FP_OUT, "w")) == NULL)
    {
        printf("Program terminated: cannot create or open output file\n");
        exit(1);
    }
    for (i = 0; i < nrow_rr_d; i++)
    {
        // iterate each target day
        df_rr_h_out.date = (p_rrd + i)->date;
        df_rr_h_out.rr_d = (p_rrd + i)->p_rr; 
        df_rr_h_out.rr_h = calloc(p_gp->N_STATION, sizeof(double) * 24);  // allocate memory (stack);

        if (SUN_dark(p_gp->N_STATION, (p_rrd + i)->p_rr) == 1)
        {
            // this is a fully cloudy day; totally dark for each site
            n_can = -1;
            for (j = 0; j < p_gp->N_STATION; j++)
            {
                for (h = 0; h < 24; h++)
                {
                    df_rr_h_out.rr_h[j][h] = 0.0;
                }
            }
            for (size_t t = 0; t < p_gp->RUN; t++)
            {
                /* write the disaggregation output */
                Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT, t + 1);
            }
            break; // next step
        }

        n_can = 0;     // number of candidates
        class_t = (p_rrd + i)->class;
        for (j = skip; j < ndays_h - skip; j++)
        {
            class_c = (p_rrh + j)->class; // the class of the candidate day
            if (class_c == class_t)       
            {
                pool_cans[n_can] = j;
                n_can += 1;
            }
        }
        
        int index = 0;  // the number of candidates after class and 0-check 
        if (p_gp->VAR == 4 || p_gp->VAR == 1)
        {
            /* check the 0 and non-zero for sunshine duration (VAR: 4) and wind speed (VAR: 1) */
            for (j = 0; j < n_can; j++)
            {
                if (SUN_zero_fit(p_gp->N_STATION, (p_rrd + i)->p_rr, (p_rrh + pool_cans[j])->rr_d) == 1)
                {
                    pool_cans[index] = pool_cans[j];
                    index += 1;
                }
            }
        } else {
            index = n_can;
        }
        n_can = index;
        Solar_MAX_filter(p_rrh, p_rrd + i, p_gp, Solar_MAX, pool_cans, &n_can);
        if (n_can == 0)
        {
            printf("No candidates for step: %d!\n", i);
            exit(1);
        }

        /*******************
         * hourly maximum check
         * ******************/
        double *SSIM;
        SSIM = (double *)malloc(n_can * sizeof(double)); // the SSIM between target day and candidate days

        int skip_temp;
        if (i >= skip && i < nrow_rr_d - skip)
        {
            skip_temp = skip;
        } else {
            skip_temp = 0;
        }
        similarity_SSIM_solar(p_rrd, p_rrh, p_gp, i, pool_cans, n_can, skip_temp, SSIM);
        
        /*******
         * sample the candidates, assign the fragments and then write the output
         * *****/
        int *index_fragment;
        index_fragment = (int *)malloc(sizeof(int) * p_gp->RUN);
        kNN_sampling(SSIM, pool_cans, order, n_can, p_gp->RUN, index_fragment);

        int size_pool; // the k in kNN
        size_pool = (int)sqrt(n_can) + 1;
        for (j = 0; j < size_pool; j++)
        {
            fprintf(p_SSIM, "%d-%02d-%02d,", (p_rrd + i)->date.y, (p_rrd + i)->date.m, (p_rrd + i)->date.d);
            fprintf(p_SSIM, "%d,%d,%f,", j, pool_cans[j], SSIM[j]);
            fprintf(p_SSIM, "%d-%02d-%02d\n", (p_rrh + pool_cans[j])->date.y, (p_rrh + pool_cans[j])->date.m, (p_rrh + pool_cans[j])->date.d);
        }
        for (size_t t = 0; t < p_gp->RUN; t++)
        {
            /*assign the sampled fragments to target day (disaggregation)*/
            Fragment_assign(p_rrh, &df_rr_h_out, p_gp, index_fragment[t]);
            /* write the disaggregation output */
            Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT, t + 1);
        }

        printf("%d-%02d-%02d: Done!\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d);
        free(df_rr_h_out.rr_h);  // free the memory allocated for disaggregated hourly output
        free(SSIM);
    }
    fclose(p_FP_OUT);
}

void Solar_MAX_derive(
    double **Solar_MAX,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int ndays_h
)
{
    /*****************
     * derive the hourly maximum solar radiation for each station
     * ****************/
    int N;
    N = p_gp->N_STATION;
    // initialize
    for (size_t i = 0; i < N; i++)
    {
        *(*Solar_MAX + i) = 0.0;
    }
    // derive the max for each site
    for (size_t i = 0; i < ndays_h; i++)
    {
        for (size_t j = 0; j < N; j++)
        {
            for (size_t h = 0; h < 24; h++)
            {
                if (
                    (p_rrh + i)->rr_h[j][h] > *(*Solar_MAX + j)
                )
                {
                    *(*Solar_MAX + j) = (p_rrh + i)->rr_h[j][h];
                }
            }
        }
    }
}

void Solar_MAX_filter(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    double *Solar_MAX,
    int *pool_cans,
    int *n_can
)
{
    int N;
    N = p_gp->N_STATION;
    double *fragments;
    double *out_temp;
    out_temp = (double *)malloc(sizeof(double) * 24);
    int flag;
    int id = 0;
    for (size_t i = 0; i < *n_can; i++)
    {
        flag = 1;
        size_t j = 0;
        while (flag == 1 && j < N)
        {
            for (size_t h = 0; h < 24; h++)
            {
                *(out_temp + h) = p_rrd->p_rr[j] * (p_rrh + pool_cans[i])->rr_h[j][h] / (p_rrh + pool_cans[i])->rr_d[j]; 
                if (*(out_temp + h) > *(Solar_MAX + j))
                {
                    flag = 0;
                    break;
                }
            } 
            j++;
        }
        if (flag == 1)
        {
            pool_cans[id] = pool_cans[i];
            id++;
        }
    }
    *n_can = id;
}


void similarity_SSIM_solar(
    struct df_rr_d *p_rrd,
    struct df_rr_h *p_rrh,
    struct Para_global *p_gp,
    int index_target,
    int *pool_cans,
    int n_can,
    int skip,
    double *SSIM
)
{
    double w_image[5] = {0.08333333, 0.1666667, 0.5, 0.1666667, 0.08333333};  // CONTUNITY == 5
    if (skip == 0)
    {
        // CONTUNITY == 1
        w_image[0] = 1.0;
    } else if (skip == 1)
    {
        // CONTUNITY == 3
        w_image[0] = 0.1666667; w_image[1] = 0.6666667; w_image[2] = 0.1666667;
    } else if (skip > 2)
    {
        printf("Currently CONTUNITY > 5 is not possible!\n");
        exit(1);
    }
    
    int i, s; // iteration variable
    int temp_c;  // temporary variable during sorting
    double temp_d;
    double SSIM_temp;
    /** compute mean-SSIM between target and candidate images **/
    for (i = 0; i < n_can; i++)
    {
        *(SSIM + i) = 0.0;
        for (s = 0-skip; s < 1+skip; s++)
        {
            if (
                p_gp->VAR == 4 &&
                (SUN_dark(p_gp->N_STATION, (p_rrd + index_target + s)->p_rr) == 1 ||
                 SUN_dark(p_gp->N_STATION, (p_rrh + pool_cans[i] + s)->rr_d) == 1))
            {
                SSIM_temp = 0.0;
            }
            else
            {
                // SSIM_temp = w_image[s + skip] * meanSSIM(
                //                                     (p_rrd + index_target + s)->p_rr,
                //                                     (p_rrh + pool_cans[i] + s)->rr_d,
                //                                     p_gp->NODATA,
                //                                     p_gp->N_STATION,
                //                                     p_gp->k,
                //                                     p_gp->power);
                SSIM_temp = w_image[s + skip] * meanSSIM(
                                                    (p_rrd + index_target + s)->p_rr_nom,
                                                    (p_rrh + pool_cans[i] + s)->rr_d_nom,
                                                    p_gp->NODATA,
                                                    p_gp->N_STATION,
                                                    p_gp->k,
                                                    p_gp->power);
            }
            *(SSIM + i) += SSIM_temp;
        }
    }
}
