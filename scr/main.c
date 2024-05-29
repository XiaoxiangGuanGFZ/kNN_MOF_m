#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "def_struct.h"
#include "Func_dataIO.h"
#include "Func_Initialize.h"
#include "Func_Prepro.h"
#include "Func_Print.h"
#include "Func_SSIM.h"
#include "Func_Disaggregate.h"
// #include "Func_Covariate.h"
#include "Func_Solar.h"

/****** exit description *****
 * void exit(int status);
 * from <stdlib.h>
 *  The exit() function takes a single argument, status,
 * which is an integer value. This status code is
 * returned to the parent process or the operating system.
 * By convention, a status code of 0 usually indicates successful execution,
 * and any other non-zero value typically indicates
 * an error or abnormal termination.
 * --------
 * 0: successfuly execution
 * 1: file input or output error
 * 2: algorithm error
 *
 */

FILE *p_SSIM;
FILE *p_log;  // file pointer pointing to log file
int f_prep;   // flag for preprocessing the data with normalization or standardization

/*****************
 * main function
 **************/
int main(int argc, char * argv[]) {
    /*
    int argc: the number of parameters of main() function;
    char *argv[]: pointer array
    */
    /* char fname[100] = "D:/kNN_MOF_cp/data/global_para.txt";
        this should be the only extern input for this program */
    time_t tm;  //datatype from <time.h>
    time(&tm);

    struct Para_global Para_df; // define the global-parameter structure
    struct Para_global *p_gp;   // give a pointer to global_parameter structure
    p_gp = &Para_df;

    /******* import the global parameters ***********
    parameter from main() function, pointer array
    argv[0]: pointing to the first string from command line (the executable file)
    argv[1]: pointing to the second string (parameter): file path and name of global parameter file.
    */
    import_global(*(++argv), p_gp);
    char VARname[20] = ""; VAR_NAME(p_gp->VAR, VARname);
    if ((p_log = fopen(p_gp->FP_LOG, "a+")) == NULL)
    {
        printf("cannot create / open log file\n");
        exit(1);
    }
    Print_gp(p_gp);
    f_prep = p_gp->PREPROCESS;
    /******* import circulation pattern series *********/
    
    static struct df_cp df_cps[MAXrow];
    int nrow_cp=0;  // the number of CP data columns: 4 (y, m, d, cp)
    if (strncmp(p_gp->T_CP, "TRUE", 4) == 0) {
        nrow_cp = import_df_cp(Para_df.FP_CP, df_cps);
        Print_cp(df_cps, nrow_cp);
    } 
    if (strncmp(p_gp->MONTH, "TRUE", 4) == 0)
    {
        time(&tm);
        printf("------ Disaggregation conditioned on 12 months: %s", ctime(&tm));
        fprintf(p_log, "------ Disaggregation conditioned on 12 months: %s", ctime(&tm));
    }
    else if (strncmp(p_gp->SEASON, "TRUE", 4) == 0)
    {
        time(&tm);
        printf("------ Disaggregation conditioned on seasonality-summer and winter: %s", ctime(&tm));
        fprintf(p_log, "------ Disaggregation conditioned on seasonality-summer and winter: %s", ctime(&tm));
    }
    
    /****** import daily rainfall data (to be disaggregated) *******/
    static struct df_rr_d df_dly[MAXrow];
    int nrow_rr_d;
    nrow_rr_d = import_dfrr_d(Para_df.FP_DAILY, Para_df.N_STATION, df_dly);
    initialize_dfrr_d(p_gp, df_dly, df_cps, nrow_rr_d, nrow_cp);
    Print_dly(df_dly, p_gp, nrow_rr_d);

    /****** import hourly rainfall data (obs as fragments) *******/
    int ndays_h;
    static struct df_rr_h df_hly[MAXrow];
    ndays_h = import_dfrr_h(p_gp->VAR, Para_df.FP_HOURLY, Para_df.N_STATION, df_hly);
    initialize_dfrr_h(p_gp, df_hly, df_cps, ndays_h, nrow_cp);
    Print_hly(df_hly, ndays_h);
    /****** preprocessing *******/
    if (f_prep == 1)
    {
        Normalize(p_gp, df_dly, df_hly, nrow_rr_d, ndays_h);
    }
    else if (f_prep == 2)
    {
        Standardize(p_gp, df_dly, df_hly, nrow_rr_d, ndays_h);
    }

    /****** covariate *******/
    // static struct df_rr_d df_dly_cov[MAXrow];
    // static struct df_rr_h df_hly_cov[MAXrow];
    // if (p_gp->VAR == 5)
    // {
    //     /****** import covariate data *******/
    //     int VAR_cov = 0;
    //     int nrow_rr_d_cov;
    //     nrow_rr_d_cov = import_dfrr_d(Para_df.FP_COV_DLY, Para_df.N_STATION, df_dly_cov);
    //     Normalize_d(p_gp, df_dly_cov, nrow_rr_d_cov);

    //     int ndays_h_cov;
    //     ndays_h_cov = import_dfrr_h(VAR_cov, Para_df.FP_COV_HLY, Para_df.N_STATION, df_hly_cov);
    //     Normalize_h(p_gp, df_hly_cov, ndays_h_cov);
        
    //     if (!(nrow_rr_d == nrow_rr_d_cov && ndays_h == ndays_h_cov))
    //     {
    //         printf("Conflict in dimension of covariate data!\n");
    //         exit(1);
    //     }
    //     time(&tm);
    //     printf("------ Import covariate data (Done): %s", ctime(&tm)); 
    //     fprintf(p_log, "------ Import covariate data (Done): %s", ctime(&tm));
    
    // }

    /****** Disaggregation: kNN_MOF_cp *******/

    if (strncmp(p_gp->FP_SSIM, "FALSE", 5) == 0)
    {
        p_SSIM = NULL;
    }
    else 
    {
        if ((p_SSIM = fopen(p_gp->FP_SSIM, "w")) == NULL)
        {
            printf("Cannot create / open SSIM file: %s\n", p_gp->FP_SSIM);
            exit(1);
        }
        fprintf(p_SSIM, "target,ID,index_Frag,SSIM,candidate\n");
    }
    
    printf("------ Disaggregating: ... \n");
    if (p_gp->VAR == 5)
    {   // VAR:5  solar radiation
        double *solar_max;
        Solar_MAX_class_derive(&solar_max, df_hly, p_gp, ndays_h);
        Solar_MAX_class_preview(solar_max, p_gp);
        kNN_MOF_solar(
            df_hly,
            df_dly,
            p_gp,
            solar_max,
            nrow_rr_d,
            ndays_h);
    } else {
        kNN_MOF_SSIM(
            df_hly,
            df_dly,
            p_gp,
            nrow_rr_d,
            ndays_h);
    }
    
    fclose(p_SSIM);
    time(&tm);
    printf("------ Disaggregation daily2hourly (Done): %s", ctime(&tm));
    fprintf(p_log, "------ Disaggregation daily2hourly (Done): %s", ctime(&tm));
    return 0; 
}



