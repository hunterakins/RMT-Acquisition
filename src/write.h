#ifndef __WRITE_H
#define __WRITE_H
#include <stdio.h>
#include <stdlib.h>

int WriteTimeData(FILE * fd, float *time_series, int len);

int WriteTwoChannels(FILE * fd, float *time_series, float *second_time_series, int len);

int WriteTimeFreqData(FILE * fd, float *time_series, float * realf, float * imf, int len);

int WriteAllData(FILE *fd, float * time_series, float * realf, float *imf, float *cp, float *icp); 


#endif
