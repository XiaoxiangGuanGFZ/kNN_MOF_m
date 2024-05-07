/*
 * SUMMARY:      Func_Fragments.c
 * USAGE:        functions related to Fragments in terms of different meteorological variables
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
 * FUNCTIONS:    SUN_zero_fit(); Fragment_assign(); 
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
#include <stdlib.h>
#include "def_struct.h"
#include "Func_Fragments.h"

int SUN_zero_fit(
    int N_STATION,
    double *target,
    double *can
)
{
    int match = 1;
    for (size_t i = 0; i < N_STATION; i++)
    {
        if (*(target + i) > 0.0 && *(can + i) <= 0.05)
        {
            match = 0;
            break;
        }
    }
    return(match);
}

int SUN_dark(
    int N_STATION,
    double *target
)
{
    int dark = 1;
    for (size_t i = 0; i < N_STATION; i++)
    {
        if (*(target + i) > 0.0)
        {
            dark = 0;  // not totally dark
            break;
        }
    }
    return(dark);
}

void Fragment_assign(
    struct df_rr_h *p_rrh,
    struct df_rr_h *p_out,
    struct Para_global *p_gp,
    int fragment
){
    /**********
     * Description:
     *      disaggregate the target day rainfall into hourly scale based on the selected fragments
     * Parameters: 
     *      p_rrh: pointing to the hourly obs rr structure array
     *      p_out: pointing to the disaggregated hourly rr results struct (to output) 
     *      p_gp: global parameters struct
     *      fragment: the index of p_rrh struct after filtering and resampling
     * Output:
     *      p_out
     * *******/
    int j, h;
    if (p_gp->VAR == 4 || p_gp->VAR == 1)
    {
        /************
         * VAR
         * - sunshine duration
         * ********/
        for (j = 0; j < p_gp->N_STATION; j++)
        {
            if (p_out->rr_d[j] <= 0.05)
            {
                // a fully cloudy day, no sunshine
                for (h = 0; h < 24; h++)
                {
                    p_out->rr_h[j][h] = 0.0;
                }
            } else {
                if ((p_rrh + fragment)->rr_d[j] <= 0.0)
                {
                    printf("fragment: %d\n", fragment);
                    exit(1);
                }
                for (h = 0; h < 24; h++)
                {
                    p_out->rr_h[j][h] = p_out->rr_d[j] * (p_rrh + fragment)->rr_h[j][h] / (p_rrh + fragment)->rr_d[j];
                    /****
                     * unit:
                     * p_out->rr_h[j][h]: minute
                     * p_out->rr_d[j]: hour
                     * (p_rrh + fragment)->rr_h[j][h]: minute
                     * (p_rrh + fragment)->rr_d[j]: hour
                     * ***/
                }
            }
        }
    }
    else if (p_gp->VAR == 0)
    {
        /********
         * - air temperature
         * *******/
        for (j = 0; j < p_gp->N_STATION; j++)
        {
            for (h = 0; h < 24; h++)
            {
                p_out->rr_h[j][h] = p_out->rr_d[j] + (p_rrh + fragment)->rr_h[j][h] - (p_rrh + fragment)->rr_d[j];
            }
        }
    }
    else
    {
        // for other weather variables: rhu, pressure
        for (j = 0; j < p_gp->N_STATION; j++)
        {
            for (h = 0; h < 24; h++)
            {
                p_out->rr_h[j][h] = p_out->rr_d[j] * (p_rrh + fragment)->rr_h[j][h] / (p_rrh + fragment)->rr_d[j];
            }
        }
    }

    /***************
     * boundary for climate variable: sunshine duration (60 min) and relative humidity 100 %
     * *********/
    if (p_gp->VAR == 3)
    {
        for (j = 0; j < p_gp->N_STATION; j++)
        {
            for (h = 0; h < 24; h++)
            {
                if (p_out->rr_h[j][h] > 100)
                {
                    p_out->rr_h[j][h] = 100;
                }
            }
        }
    }
    else if (p_gp->VAR == 4)
    {
        for (j = 0; j < p_gp->N_STATION; j++)
        {
            for (h = 0; h < 24; h++)
            {
                if (p_out->rr_h[j][h] > 60)
                {
                    p_out->rr_h[j][h] = 60;
                }
            }
        }
    }
}

