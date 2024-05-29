/*
 * SUMMARY:      Func_Disaggregate.c
 * USAGE:        main algorithm of method-of-fragments for temporal disaggregation
 * AUTHOR:       Xiaoxiang Guan
 * ORG:          Section Hydrology, GFZ
 * E-MAIL:       guan@gfz-potsdam.de
 * ORIG-DATE:    Apr-2024
 * DESCRIPTION:  the method-of-fragments conditioned on :
 *               - circulation patterns (classification)
 *               - seasonality (summer or winter)
 *               - the similarity is represented by SSIM (structural Similarity Index Measure)
 *               - kNN is used to consider the uncertainty or variability 
 * DESCRIP-END.
 * FUNCTIONS:    kNN_MOF_SSIM();  kNN_SSIM_sampling(); Rhu_MAX_class_filter();
 * 
 * COMMENTS:
 * 
 * REFERENCEs:
 * 
 */

/*******************************************************************************
 * VARIABLEs:
 * 
*****/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "def_struct.h"
#include "Func_Fragments.h"
#include "Func_kNN.h"
#include "Func_SSIM.h"
#include "Func_Disaggregate.h"
#include "Func_dataIO.h"

void kNN_MOF_SSIM(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
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
    int i, j, h, k, s;
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
    int n_can;             // the number of candidates after all conditioning (cp and seasonality)
    int fragment;          // the index of df_rr_h structure with the final chosed fragments

    FILE *p_FP_OUT;
    if ((p_FP_OUT=fopen(p_gp->FP_OUT, "w")) == NULL) {
        printf("Program terminated: cannot create or open output file\n");
        exit(1);
    }
    for (i = 0; i < nrow_rr_d; i++)
    {
        // iterate each target day
        df_rr_h_out.date = (p_rrd + i)->date;
        df_rr_h_out.rr_d = (p_rrd + i)->p_rr; 
        df_rr_h_out.rr_h = calloc(p_gp->N_STATION, sizeof(double) * 24);  // allocate memory (stack);
        
        if (p_gp->VAR == 4)  // sunshine duration
        {
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
                break; // next day
            }
        }

        n_can = 0;     // number of candidates
        class_t = (p_rrd + i)->class;
        for (size_t j = skip; j < ndays_h - skip; j++)
        {
            class_c = (p_rrh + j)->class; // the class of the candidate day
            if (class_c == class_t)       
            {
                pool_cans[n_can] = j;
                n_can += 1;
            }
        }
        
        int index = 0;
        if (p_gp->VAR == 4 || p_gp->VAR == 1)
        {
            /* check the 0 and non-zero for sunshine duration and wind speed */
            for (j = 0; j < n_can; j++)
            {
                if (SUN_zero_fit(p_gp->N_STATION, (p_rrd + i)->p_rr, (p_rrh + pool_cans[j])->rr_d) == 1)
                {
                    pool_cans[index] = pool_cans[j];
                    index += 1;
                }
            }
            n_can = index;
        }
        
        if (p_gp->VAR == 3)
        {
            index = 0;
            // VAR 3: rhu, no greater than 100%
            Rhu_MAX_class_filter(p_rrh, p_rrd + i, p_gp, pool_cans, n_can, &index);
        }
        
        // index:  the number of candidates after filtering
        // if (index == 0)
        // {
        //     printf("No candidates for step: %d!\n", i);
        //     exit(1);
        // }
        if (index > 0)
        {
            n_can = index;
        }
        
        int *index_fragment;
        index_fragment = (int *)malloc(sizeof(int) * p_gp->RUN);
        if (i >= skip && i < nrow_rr_d - skip)
        {
            kNN_SSIM_sampling(p_rrd, p_rrh, p_gp, i, pool_cans, order, n_can, skip, p_gp->RUN, index_fragment);
        }
        else
        {
            kNN_SSIM_sampling(p_rrd, p_rrh, p_gp, i, pool_cans, order, n_can, 0, p_gp->RUN, index_fragment);
        }
        /*assign the sampled fragments to target day (disaggregation)*/
        for (size_t t = 0; t < p_gp->RUN; t++)
        {
            Fragment_assign(p_rrh, &df_rr_h_out, p_gp, index_fragment[t]);
            /* write the disaggregation output */
            Write_df_rr_h(&df_rr_h_out, p_gp, p_FP_OUT, t + 1);
        }

        printf("%d-%02d-%02d: Done!\n", (p_rrd+i)->date.y, (p_rrd+i)->date.m, (p_rrd+i)->date.d);
        free(df_rr_h_out.rr_h);  // free the memory allocated for disaggregated hourly output
    }
    fclose(p_FP_OUT);
}


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
){
    /**************
     * Description:
     *      - compute the SSIM index of rr between target and candidate days
     *      - sort the distance in the decreasing order
     *      - select the sqrt(n_can) largest SSIM
     *      - weights defined based on SSIM: higher SSIM, heavier weight
     *      - sample one candidate 
     * Parameters:
     *      p_rrd: the daily rainfall st (structure pointer)
     *      p_rrh: pointing to the hourly rr obs structure array
     *      p_gp: pointing to global parameter structure
     *      index_target: the index of target day to be disaggregated
     *      pool_cans: the index pool of candidats
     *      n_can: the number (or size) fo candidates pool
     *      skip: due to the consideration of days before and after the target day, 
     *              the first and last several days should be disaggregated by assuming CONTUNITY == 1
     * Output:
     *      return a vector (number) of sampled index (fragments source); how many RUNs of sampling
     * ***********/
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

    int i, j, s; // iteration variable
    int temp_c;  // temporary variable during sorting
    double temp_d;
    double rd = 0.0; // a random decimal value between 0.0 and 1.0

    double *SSIM; // the distance between target day and candidate days
    double SSIM_temp;
    SSIM = (double *)malloc(n_can * sizeof(double));

    /** compute mean-SSIM between target and candidate images **/
    if (f_prep == 0)
    {
        for (i = 0; i < n_can; i++)
        {
            *(SSIM + i) = 0.0;
            for (s = 0 - skip; s < 1 + skip; s++)
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
                    SSIM_temp = w_image[s + skip] * meanSSIM(
                                                        (p_rrd + index_target + s)->p_rr,
                                                        (p_rrh + pool_cans[i] + s)->rr_d,
                                                        p_gp->NODATA,
                                                        p_gp->N_STATION,
                                                        p_gp->k,
                                                        p_gp->power);
                }
                *(SSIM + i) += SSIM_temp;
            }
        }
    }
    else
    {
        for (i = 0; i < n_can; i++)
        {
            *(SSIM + i) = 0.0;
            for (s = 0 - skip; s < 1 + skip; s++)
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
                    SSIM_temp = w_image[s + skip] * meanSSIM(
                                                        (p_rrd + index_target + s)->p_rr_pre,
                                                        (p_rrh + pool_cans[i] + s)->p_rr_pre,
                                                        p_gp->NODATA,
                                                        p_gp->N_STATION,
                                                        p_gp->k,
                                                        p_gp->power);
                }
                *(SSIM + i) += SSIM_temp;
            }
        }
    }

    // sort the SSIM in the decreasing order
    kNN_sampling(SSIM, pool_cans, order, n_can, run, index_fragment);

    /**********
     * print the largest k SSIM values and corresponding candidate dates
     * ********/
    if (p_SSIM != NULL)
    {
        int size_pool; // the k in kNN
        size_pool = (int)sqrt(n_can) + 1;
        for (i = 0; i < size_pool; i++)
        {
            fprintf(p_SSIM, "%d-%02d-%02d,", (p_rrd + index_target)->date.y, (p_rrd + index_target)->date.m, (p_rrd + index_target)->date.d);
            fprintf(p_SSIM, "%d,%d,%f,", i, pool_cans[i], SSIM[i]);
            fprintf(p_SSIM, "%d-%02d-%02d\n", (p_rrh + pool_cans[i])->date.y, (p_rrh + pool_cans[i])->date.m, (p_rrh + pool_cans[i])->date.d);
        }
    }
    
    free(SSIM);
}


void Rhu_MAX_class_filter(
    struct df_rr_h *p_rrh,
    struct df_rr_d *p_rrd,
    struct Para_global *p_gp,
    int *pool_cans,
    int n_can,
    int *n_can_out
)
{
    double rhu_max = 120;
    int N;
    int class_t;
    int CLASS_N;
    class_t = p_rrd->class;
    CLASS_N = p_gp->CLASS_N;

    N = p_gp->N_STATION;
    double *out_temp;
    out_temp = (double *)malloc(sizeof(double) * 24);
    int flag;
    int id = 0;
    for (size_t i = 0; i < n_can; i++)
    {
        flag = 1;
        size_t j = 0;
        while (flag == 1 && j < N)
        {
            for (size_t h = 0; h < 24; h++)
            {
                *(out_temp + h) = p_rrd->p_rr[j] * (p_rrh + pool_cans[i])->rr_h[j][h] / (p_rrh + pool_cans[i])->rr_d[j]; 
                if (*(out_temp + h) > rhu_max)
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
    *n_can_out = id;
}
