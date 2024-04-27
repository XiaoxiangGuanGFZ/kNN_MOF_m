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
    int nrow_rr_d
)
{
    /******************
     * time series is classified based on 
     * - month
     * - season
     * - neither
     * ****/
    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) == 0) 
    {
        printf("The disaggregation can only be conditioned on either MONTH or SEASON!\n");
        exit(1);
    }
    
    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) != 0)
    {
        /***********
         * assign each day the month value
         ***********/
        p_gp->CLASS_N = 12;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->class = (p_rr_d + i)->date.m - 1;  // starting from 0
        }
    } 
    else if (strncmp(p_gp->SEASON, "TRUE", 4) == 0) {
        /*******
         * assign each day the season (summer or winter) value
         ****/
        p_gp->CLASS_N = 2;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_d + i)->date.m >= p_gp->SUMMER_FROM && (p_rr_d + i)->date.m <= p_gp->SUMMER_TO)
            {
                (p_rr_d + i)->class = 1; // summer
            } else {
                (p_rr_d + i)->class = 0; // winter
            }
        }
    } else {
        p_gp->CLASS_N = 1;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_d + i)->class = 0;
        }
    }

}


void initialize_dfrr_h(
    struct Para_global *p_gp,
    struct df_rr_h *p_rr_h,
    int nrow_rr_d
)
{
    /******************
     * time series is classified based on
     * - month
     * - season
     * - neither
     * ****/
    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) == 0)
    {
        printf("The disaggregation can only be conditioned on either MONTH or SEASON!\n");
        exit(1);
    }

    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0 && strncmp(p_gp->SEASON, "TRUE", 4) != 0)
    {
        /***********
         * assign each day the month value
         ***********/
        p_gp->CLASS_N = 12;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->class = (p_rr_h + i)->date.m - 1; // starting from 0
        }
    }
    else if (strncmp(p_gp->SEASON, "TRUE", 4) == 0)
    {
        /*******
         * assign each day the season (summer or winter) value
         ****/
        p_gp->CLASS_N = 2;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            if ((p_rr_h + i)->date.m >= p_gp->SUMMER_FROM && (p_rr_h + i)->date.m <= p_gp->SUMMER_TO)
            {
                (p_rr_h + i)->class = 1; // summer
            }
            else
            {
                (p_rr_h + i)->class = 0; // winter
            }
        }
    }
    else
    {
        p_gp->CLASS_N = 1;
        for (size_t i = 0; i < nrow_rr_d; i++)
        {
            (p_rr_h + i)->class = 0;
        }
    }
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

// void initialize_df_coor(
//     struct Para_global *p_gp,
//     struct df_coor **p_coor,
//     int nrow_coor
// )
// {
//     if (nrow_coor != p_gp->N_STATION)
//     {
//         printf("FP_COOR: rainsite number contradiction between FP_COOR and data files!\n");
//         exit(1);
//     } else {
//         double *distance; double d_temp;
//         int *id; int id_temp;
//         int n_neighbors;
//         n_neighbors = nrow_coor - 1;  // number of neighbouring rain site in total
//         distance = (double *)malloc(sizeof(double) * n_neighbors);
//         id = (int *)malloc(sizeof(int) * n_neighbors);

//         /**************
//          * iterate each rain site, obtain a rank vector of
//          * distance with all the neighbours (in increasing order)
//          */
//         for (size_t i = 0; i < nrow_coor; i++)
//         {
//             (*p_coor + i)->neighbors = (int *)malloc(sizeof(int) * n_neighbors);

//             /*****
//              * calculate the distance between target rain site with its neighbours
//              * every rain site has n-1 neighbours (excluding itself)
//              */
//             int t = 0;
//             for (size_t j = 0; j < nrow_coor; j++)
//             {
//                 if (i != j)
//                 {
//                     *(distance + t) = COOR_distance(
//                         (*p_coor + i)->lon, (*p_coor + i)->lat, 
//                         (*p_coor + j)->lon, (*p_coor + j)->lat
//                     );
//                     *(id + t) = j;
//                     t += 1;
//                 }                
//             }

//             // sorting the distance vector in the increasing order
//             for (size_t j = 0; j < n_neighbors - 1; j++)
//             {
//                 for (size_t k = j + 1; k < n_neighbors; k++)
//                 {
//                     if (*(distance + j) > *(distance + k))
//                     {
//                         d_temp = *(distance + j);
//                         *(distance + j) = *(distance + k);
//                         *(distance + k) = d_temp;
//                         id_temp = *(id + j);
//                         *(id + j) = *(id + k);
//                         *(id + k) = id_temp;
//                     }
//                 }
//             }

//             // assign the rank to the structure
//             for (size_t j = 0; j < n_neighbors; j++)
//             {
//                 *((*p_coor + i)->neighbors + j) = *(id + j);
//             }
//         }
//         free(id);
//         free(distance);
//     }
    
// }

// double COOR_distance(
//     double lon1,
//     double lat1,
//     double lon2,
//     double lat2
// )
// {
//     double d;
//     d = pow((lon1 - lon2), 2) + pow(lat1 - lat2, 2);
//     return d;
// }
