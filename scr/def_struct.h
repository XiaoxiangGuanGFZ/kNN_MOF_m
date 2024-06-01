#ifndef STRUCT_H
#define STRUCT_H

#define MAXCHAR 10000  // able to accomodate up to 3000 sites simultaneously
#define MAXrow 100000  // almost 270 years long ts
#define MAXcps 20
/******
 * the following define the structures
*/
struct Date {
    int y;
    int m;
    int d;
};

struct df_rr_d
{
    /* data
     * data frame for the daily step precipitation,
     * p_rr points to a double-type array, 
     *      with the size equal to the number of stations 
     */
    struct Date date;    
    double *p_rr;
    double *p_rr_pre;  // data series at daily scale after preprocessing
    int cp;
    int SM;         // summer or winter; 1 or 0
    int class;      // class of the day; categorized by cp, seaspn, month or ... 
};

struct df_rr_h
{
    /* data
     * dataframe for the hourly step precipitation, 
     * rr_h: pointer array; 
     *      24 double-type pointers; 
     *      each points to an array of hourly precipitation (all rain sites)
     * rr_d: double-type pointer;
     *      pointing to an array of daily precipitation (all rain site) aggregated from rr_h
     */
    struct Date date;    
    double (*rr_h)[24];
    double *rr_d;     // daily data aggregated from hourly; (*rr_h)[24]
    double *p_rr_pre; // daily data after preprocessing
    int cp;
    int SM;
    int class;
};

struct df_cp
{
    /* 
     * circulation pattern classficiation series
     * each day with a CP class
     */
    struct Date date;    
    int cp;
};

struct Para_global
    {
        /* global parameters */
        int VAR;                // meteorological variable
        /**************
         * VAR  weather field
         * 0    air_temperature
         * 1    wind speed
         * 2    air pressure
         * 3    relative humidity
         * 4    sunshine duration
         * 5    solar radiation
         * ********/
        
        char FP_DAILY[200];     // file path of daily precipitation data (to be disaggregated)
        char FP_CP[200];        // file path of circulation pattern (CP) classification data series
        char FP_HOURLY[200];    // file path of hourly precipitation data (as fragments)
        char FP_OUT[200];       // file path of output(hourly) precipitation from disaggregation
        char FP_LOG[200];       // file path of log file
        char FP_SSIM[200];      // file path and name to SSIM output
        /*****
         * the covariate (both daily and hourly) data should share the 
         * same dimension (time coverage and space or sites domain) with 
         * the variable data to be disaggregated
         * ****/
        char FP_COV_DLY[200];   // file path and name of daily covariate data
        char FP_COV_HLY[200];   // file path and name of hourly covariate observation data

        char SIMILARITY[10];    // the similarity index: Manhattan or SSIM

        int N_STATION;          // number of stations (rain sites)
        char T_CP[10];          // toggle (flag), whether the CP is considered in the algorithm

        char MONTH[10];         // toggle (flag), conditioned on month: 12 months
        char SEASON[10];        // toggle (flag), whether the seasonality is considered in the algorithm
        int SUMMER_FROM;        // the beginning month of summer
        int SUMMER_TO;          // the end month of summer

        int CONTINUITY;         // continuity day
        int CLASS_N;            // total categories the series is classified into

        double k[3];            // 3 parameters in SSIM
        double power[3];        // 3 paras in SSIM
        double NODATA;          // nodata value
        int RUN;                // simulation runs 
        int PREPROCESS;         // preprocess the data by normalization or standardization
        /**********
         * PREPROCESS:
         * 0: none
         * 1: normalization
         * 2: standardization
         * ********/
    };


#endif
