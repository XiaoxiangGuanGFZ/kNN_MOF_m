#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "def_struct.h"
#include "Func_dataIO.h"
#include "Func_Initialize.h"
#include "Func_SSIM.h"
#include "Func_Disaggregate.h"
#include "Func_Covariate.h"

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

/***************************************
 * main function 
*/
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
    char VARname[20]; VAR_NAME(p_gp->VAR, VARname);
    if ((p_log = fopen(p_gp->FP_LOG, "a+")) == NULL)
    {
        printf("cannot create / open log file\n");
        exit(1);
    }
    printf("------ Global parameter import completed: %s", ctime(&tm));
    fprintf(p_log, "------ Global parameter import completed: %s", ctime(&tm));

    printf(
        "FP_DAILY: %s\nFP_HOULY: %s\nFP_OUT:   %s\nFP_LOG:   %s\n",
        p_gp->FP_DAILY, p_gp->FP_HOURLY, p_gp->FP_OUT, p_gp->FP_LOG
    );
    fprintf(p_log, "FP_DAILY: %s\nFP_HOULY: %s\nFP_OUT:   %s\nFP_LOG:   %s\n",
        p_gp->FP_DAILY, p_gp->FP_HOURLY, p_gp->FP_OUT, p_gp->FP_LOG);

    if (p_gp->VAR == 5)
    {
        printf(
            "FP_COV_DLY: %s\nFP_COV_HLY: %s\n",
            p_gp->FP_COV_DLY, p_gp->FP_COV_HLY);
        fprintf(p_log, "FP_COV_DLY: %s\nFP_COV_HLY: %s\n",
                p_gp->FP_COV_DLY, p_gp->FP_COV_HLY);
    }

    printf(
        "------ Disaggregation parameters: -----\nVAR: %s\nMONTH: %s\nN_STATION: %d\nCONTINUITY: %d\nSEASON: %s\n",
        VARname, p_gp->MONTH, p_gp->N_STATION, p_gp->CONTINUITY, p_gp->SEASON
    );
    fprintf(
        p_log,
        "------ Disaggregation parameters: -----\nVAR: %s\nMONTH: %s\nN_STATION: %d\nCONTINUITY: %d\nSEASON: %s\n",
        VARname, p_gp->MONTH, p_gp->N_STATION, p_gp->CONTINUITY, p_gp->SEASON
    );
    if (strncmp(p_gp->SEASON, "TRUE", 4) == 0)
    {
        printf("SUMMER: %d-%d\n", p_gp->SUMMER_FROM, p_gp->SUMMER_TO);
        fprintf(p_log,"SUMMER: %d-%d\n", p_gp->SUMMER_FROM, p_gp->SUMMER_TO);
    }
    printf("SSIM_K: %f,%f,%f\nSSIM_power: %f,%f,%f\nNODATA: %f\n",
           p_gp->k[0], p_gp->k[1], p_gp->k[2], p_gp->power[0], p_gp->power[1], p_gp->power[2],
           p_gp->NODATA);
    fprintf(p_log, "SSIM_K: %f,%f,%f\nSSIM_power: %f,%f,%f\nNODATA: %f\n",
            p_gp->k[0], p_gp->k[1], p_gp->k[2], p_gp->power[0], p_gp->power[1], p_gp->power[2],
            p_gp->NODATA);

    /******* import circulation pattern series *********/
    
    static struct df_cp df_cps[MAXrow];
    int nrow_cp=0;  // the number of CP data columns: 4 (y, m, d, cp)
    if (strncmp(p_gp->T_CP, "TRUE", 4) == 0) {
        nrow_cp = import_df_cp(Para_df.FP_CP, df_cps);
        time(&tm);
        printf("------ Import CP data series (Done): %s", ctime(&tm)); 
        fprintf(p_log, "------ Import CP data series (Done): %s", ctime(&tm));

        printf("* number of CP data rows: %d\n", nrow_cp); 
        fprintf(p_log, "* number of CP data rows: %d\n", nrow_cp);

        printf("* the first day: %d-%02d-%02d \n", df_cps[0].date.y, df_cps[0].date.m, df_cps[0].date.d);
        fprintf(p_log, "* the first day: %d-%02d-%02d \n", df_cps[0].date.y, df_cps[0].date.m, df_cps[0].date.d);
        
        printf("* the last day:  %d-%02d-%02d \n", 
            df_cps[nrow_cp-1].date.y, df_cps[nrow_cp-1].date.m, df_cps[nrow_cp-1].date.d
        );
        fprintf(p_log, "* the last day:  %d-%02d-%02d \n", 
            df_cps[nrow_cp-1].date.y, df_cps[nrow_cp-1].date.m, df_cps[nrow_cp-1].date.d
        );
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
    
    static struct df_rr_d df_rr_daily[MAXrow];
    int nrow_rr_d;
    nrow_rr_d = import_dfrr_d(Para_df.FP_DAILY, Para_df.N_STATION, df_rr_daily);
    initialize_dfrr_d(p_gp, df_rr_daily, df_cps, nrow_rr_d, nrow_cp);
    Normalize_d(
        p_gp,
        df_rr_daily,
        nrow_rr_d);
    // for (size_t i = 0; i < p_gp->N_STATION; i++)
    // {
    //     printf(
    //         "%f, ", (df_rr_daily + 180)->p_rr_nom[i]
    //     );
    // }
    // exit(3);

    time(&tm);
    printf("------ Import daily data (Done): %s", ctime(&tm)); fprintf(p_log, "------ Import daily data (Done): %s", ctime(&tm));
    
    printf("* the total rows: %d\n", nrow_rr_d); fprintf(p_log, "* the total rows: %d\n", nrow_rr_d);
    
    printf("* the first day: %d-%02d-%02d\n", df_rr_daily[0].date.y,df_rr_daily[0].date.m,df_rr_daily[0].date.d);
    fprintf(p_log, "* the first day: %d-%02d-%02d\n", df_rr_daily[0].date.y,df_rr_daily[0].date.m,df_rr_daily[0].date.d);

    printf(
        "* the last day:  %d-%02d-%02d\n", 
        df_rr_daily[nrow_rr_d-1].date.y,df_rr_daily[nrow_rr_d-1].date.m,df_rr_daily[nrow_rr_d-1].date.d
    );
    fprintf(
        p_log,
        "* the last day:  %d-%02d-%02d\n", 
        df_rr_daily[nrow_rr_d-1].date.y,df_rr_daily[nrow_rr_d-1].date.m,df_rr_daily[nrow_rr_d-1].date.d
    );

    view_class_rrd(df_rr_daily, nrow_rr_d);

    /****** import hourly rainfall data (obs as fragments) *******/
    
    int ndays_h;
    static struct df_rr_h df_rr_hourly[MAXrow];
    ndays_h = import_dfrr_h(p_gp->VAR, Para_df.FP_HOURLY, Para_df.N_STATION, df_rr_hourly);
    Normalize_h(
        p_gp,
        df_rr_hourly,
        ndays_h);

    initialize_dfrr_h(p_gp, df_rr_hourly, df_cps, ndays_h, nrow_cp);
    time(&tm);
    printf("------ Import hourly data (Done): %s", ctime(&tm)); fprintf(p_log, "------ Import hourly data (Done): %s", ctime(&tm));
    
    printf("* total hourly obs days: %d\n", ndays_h); fprintf(p_log, "* total hourly obs days: %d\n", ndays_h);
    
    printf("* the first day: %d-%02d-%02d\n", df_rr_hourly[0].date.y, df_rr_hourly[0].date.m, df_rr_hourly[0].date.d);
    fprintf(p_log, "* the first day: %d-%02d-%02d\n", df_rr_hourly[0].date.y, df_rr_hourly[0].date.m, df_rr_hourly[0].date.d);
    
    printf(
        "* the last day:  %d-%02d-%02d\n", 
        df_rr_hourly[ndays_h-1].date.y, df_rr_hourly[ndays_h-1].date.m, df_rr_hourly[ndays_h-1].date.d
    );
    fprintf(
        p_log,
        "* the last day:  %d-%02d-%02d\n", 
        df_rr_hourly[ndays_h-1].date.y, df_rr_hourly[ndays_h-1].date.m, df_rr_hourly[ndays_h-1].date.d
    );
    view_class_rrh(df_rr_hourly, ndays_h);
    /****** maxima of rainfall value (L value in SSIM algorithm) *******/
    // initialize_L(df_rr_hourly, df_rr_daily, p_gp, nrow_rr_d, ndays_h);

    static struct df_rr_d df_rr_daily_cov[MAXrow];
    static struct df_rr_h df_rr_hourly_cov[MAXrow];
    if (p_gp->VAR == 5)
    {
        /****** import covariate data *******/
        int VAR_cov = 0;
        int nrow_rr_d_cov;
        nrow_rr_d_cov = import_dfrr_d(Para_df.FP_COV_DLY, Para_df.N_STATION, df_rr_daily_cov);
        Normalize_d(p_gp, df_rr_daily_cov, nrow_rr_d_cov);

        int ndays_h_cov;
        ndays_h_cov = import_dfrr_h(VAR_cov, Para_df.FP_COV_HLY, Para_df.N_STATION, df_rr_hourly_cov);
        Normalize_h(p_gp, df_rr_hourly_cov, ndays_h_cov);
        
        if (!(nrow_rr_d == nrow_rr_d_cov && ndays_h == ndays_h_cov))
        {
            printf("Conflict in dimension of covariate data!\n");
            exit(1);
        }
        time(&tm);
        printf("------ Import covariate data (Done): %s", ctime(&tm)); 
        fprintf(p_log, "------ Import covariate data (Done): %s", ctime(&tm));
    
    }

    /****** Disaggregation: kNN_MOF_cp *******/

    if ((p_SSIM = fopen("D:/kNN_MOF_m/SSIM.csv", "w")) == NULL)
    {
        printf("Cannot create / open SSIM file\n");
        exit(1);
    }
    fprintf(p_SSIM, "target,ID,index_Frag,SSIM,candidate\n");
    printf("------ Disaggregating: ... \n");
    if (p_gp->VAR == 5)
    {
        kNN_MOF_cov(
            df_rr_hourly,
            df_rr_daily,
            df_rr_hourly_cov,
            df_rr_daily_cov,
            p_gp,
            nrow_rr_d,
            ndays_h);
    } else {
        kNN_MOF_SSIM(
            df_rr_hourly,
            df_rr_daily,
            p_gp, // the pointer pointing to Para_df structure;
            nrow_rr_d,
            ndays_h);
    }
    
    fclose(p_SSIM);
    time(&tm);
    printf("------ Disaggregation daily2hourly (Done): %s", ctime(&tm));
    fprintf(p_log, "------ Disaggregation daily2hourly (Done): %s", ctime(&tm));
    return 0; 
}



