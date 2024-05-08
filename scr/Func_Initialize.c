/*
 * SUMMARY:      Func_Initialize.c
 * USAGE:        assign the seasonality, and class 
 * AUTHOR:       Xiaoxiang Guan
 * ORG:          Section Hydrology, GFZ
 * E-MAIL:       guan@gfz-potsdam.de
 * ORIG-DATE:    Apr-2024
 * DESCRIPTION:  MOD is based several conditions: seasonality, month
 *               therefore, we assign each day a season (summer or winter) and a month
 * DESCRIP-END.
 * FUNCTIONS:    initialize_dfrr_d(); initialize_dfrr_h(); 
 * COMMENTS:
 * 
 *
 */

/************
 * 
 * 
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "def_struct.h"
#include "Func_Initialize.h"

void initialize_dfrr_d(
    struct Para_global *p_gp,
    struct df_rr_d *p_rr_d,
    struct df_cp *p_cp,
    int nrow_rr_d,
    int nrow_cp
)
{
    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) == 0) 
    {
        printf("The disaggregation can only be conditioned on either MONTH or SEASON!\n");
        exit(1);
    }

    /*******
     * assign each day the cp value
     */
    int N_CP_CLASS;
    if (strncmp(p_gp->T_CP, "TRUE", 4) == 0)
    {
        N_CP_CLASS = CP_classes(p_cp, nrow_cp);
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->cp = Toogle_CP((p_rr_d + i)->date, p_cp, nrow_cp);
        }
    }
    else
    {
        N_CP_CLASS = 0;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->cp = 0;
        }
    }

    /*******
     * assign each day the season (summer or winter) value
    */
    if (strncmp(p_gp->SEASON, "TRUE", 4) == 0) {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_d + i)->date.m >= p_gp->SUMMER_FROM && (p_rr_d + i)->date.m <= p_gp->SUMMER_TO)
            {
                (p_rr_d + i)->SM = 1; // summer
            } else {
                (p_rr_d + i)->SM = 0; // winter
            }
        }
    }
    else if (strncmp(p_gp->MONTH, "TRUE", 4) == 0)
    {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->SM = (p_rr_d + i)->date.m - 1;
        }
    } else {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->SM = 0;
        }
    }

    /******************
     * time series is classified based on following combinations: 
     * - month alone
     * - season alone
     * - month and cp
     * - season and cp
     * - cp alone
     * - nothing
     * ****/
    for (size_t i = 0; i < nrow_rr_d; i++)
    {
        (p_rr_d + i)->class = ((p_rr_d + i)->cp - 1) * N_CP_CLASS + (p_rr_d + i)->SM;
    }

}


void initialize_dfrr_h(
    struct Para_global *p_gp,
    struct df_rr_h *p_rr_h,
    struct df_cp *p_cp,
    int nrow_rr_d,
    int nrow_cp
)
{
        if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) == 0) 
    {
        printf("The disaggregation can only be conditioned on either MONTH or SEASON!\n");
        exit(1);
    }

    /*******
     * assign each day the cp value
     */
    int N_CP_CLASS;
    if (strncmp(p_gp->T_CP, "TRUE", 4) == 0)
    {
        N_CP_CLASS = CP_classes(p_cp, nrow_cp);
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->cp = Toogle_CP((p_rr_h + i)->date, p_cp, nrow_cp);
        }
    }
    else
    {
        N_CP_CLASS = 0;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->cp = 0;
        }
    }

    /*******
     * assign each day the season (summer or winter) value
    */
    if (strncmp(p_gp->SEASON, "TRUE", 4) == 0) {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_h + i)->date.m >= p_gp->SUMMER_FROM && (p_rr_h + i)->date.m <= p_gp->SUMMER_TO)
            {
                (p_rr_h + i)->SM = 1; // summer
            } else {
                (p_rr_h + i)->SM = 0; // winter
            }
        }
    }
    else if (strncmp(p_gp->MONTH, "TRUE", 4) == 0)
    {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->SM = (p_rr_h + i)->date.m - 1;
        }
    } else {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->SM = 0;
        }
    }

    /******************
     * time series is classified based on following combinations: 
     * - month alone
     * - season alone
     * - month and cp
     * - season and cp
     * - cp alone
     * - nothing
     * ****/
    for (size_t i = 0; i < nrow_rr_d; i++)
    {
        (p_rr_h + i)->class = ((p_rr_h + i)->cp - 1) * N_CP_CLASS + (p_rr_h + i)->SM;
    }

}


int Toogle_CP(
    struct Date date,
    struct df_cp *p_cp,
    int nrow_cp
)
{
    /*************
     * Description:
     *      derive the cp value (class) of the day based on date stamp (y-m-d)
     * Parameters:
     *      date: a Date struct, conaining y, m and d
     *      p_cp: pointing to the cp data struct array
     *      nrow_cp: total rows of cp observations
     * Output:
     *      return the derived cp value
     * **********/
    int i;
    int cp = -1;
    for (i = 0; i < nrow_cp; i++) {
        if (
            (p_cp+i)->date.y == date.y && (p_cp+i)->date.m == date.m && (p_cp+i)->date.d == date.d
        ) {
            cp = (p_cp+i)->cp;
            break; // terminate the loop directly
        }
    }
    if (cp == -1) {
        printf(
            "Program terminated: cannot find the cp class for the date %d-%02d-%02d\n",
            date.y, date.m, date.d
        );
        exit(2);
    }
    return cp;
}

int CP_classes(
    struct df_cp *p_cp,
    int nrow_cp
)
{
    int cp_max=0;
    for (size_t i = 0; i < nrow_cp; i++)
    {
        if ((p_cp + i)->cp > cp_max)
        {
            cp_max = (p_cp + i)->cp;
        }
    }
    return cp_max;
}


void view_class_rrd(
    struct df_rr_d *p_rr_d,
    int nrow_rr_d
)
{
    int n_classes = 0;
    for (size_t i = 0; i < nrow_rr_d; i++)
    {
        if ((p_rr_d + i)->class > n_classes)
        {
            n_classes = (p_rr_d + i)->class;
        }
    }
    n_classes += 1;  // the total number of classes the time series is categorized into. 

    int *counts;
    counts = (int *)malloc(sizeof(int) * n_classes);
    for (size_t i = 0; i < n_classes; i++)
    {
        // initialize 
        *(counts + i) = 0;
    }

    for (size_t t = 0; t < n_classes; t++)
    {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_d + i)->class == t)
            {
                *(counts + t) += 1;
            }
        }
    }
    
    /**********************************
     * print the counts of each class to screen
     */
    printf("* class-counts:\n   - class: "); fprintf(p_log, "* class-counts:\n   - class: ");
    for (size_t t = 0; t < n_classes; t++)
    {
        printf("%5d ", t + 1); fprintf(p_log, "%5d ", t + 1);
    }
    printf("\n"); fprintf(p_log, "\n");

    printf("   - count: "); fprintf(p_log, "   - count: ");
    for (size_t t = 0; t < n_classes; t++)
    {
        printf("%5d ", *(counts + t)); fprintf(p_log, "%5d ", *(counts + t));
    }
    printf("\n"); fprintf(p_log, "\n");
    
}


void view_class_rrh(
    struct df_rr_h *p_rr_h,
    int nrow_rr_d
)
{
    int n_classes = 0;
    for (size_t i = 0; i < nrow_rr_d; i++)
    {
        if ((p_rr_h + i)->class > n_classes)
        {
            n_classes = (p_rr_h + i)->class;
        }
    }
    n_classes += 1;  // the total number of classes the time series is categorized into. 

    int *counts;
    counts = (int *)malloc(sizeof(int) * n_classes);
    for (size_t i = 0; i < n_classes; i++)
    {
        // initialize 
        *(counts + i) = 0;
    }

    for (size_t t = 0; t < n_classes; t++)
    {
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_h + i)->class == t)
            {
                *(counts + t) += 1;
            }
        }
    }
    
    /**********************************
     * print the counts of each class to screen
     */
    printf("* class-counts:\n   - class: "); fprintf(p_log, "* class-counts:\n   - class: ");
    for (size_t t = 0; t < n_classes; t++)
    {
        printf("%5d ", t + 1); fprintf(p_log, "%5d ", t + 1);
    }
    printf("\n"); fprintf(p_log, "\n");

    printf("   - count: "); fprintf(p_log, "   - count: ");
    for (size_t t = 0; t < n_classes; t++)
    {
        printf("%5d ", *(counts + t)); fprintf(p_log, "%5d ", *(counts + t));
    }
    printf("\n"); fprintf(p_log, "\n");
}

