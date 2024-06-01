
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "def_struct.h"
#include "Func_MD.h"


double Manhattan_distance(
    double *target,
    double *candidate,
    int n
) 
{
    double dis = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        dis += abs(*(target + i) - *(candidate + i));
    }
    return(dis);
}



