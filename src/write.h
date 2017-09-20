#ifndef __WRITE_H
#define __WRITE_H
#include <stdio.h>
#include <stdlib.h>

int WriteTimeData(FILE * fd, double *time_series, int len);

int WriteTimeFreqData(FILE * fd, int16_t *time_series, int16_t * realf, int16_t * imf, int len);

int WriteAllData(FILE *fd, int16_t * time_series, int16_t * realf, int16_t *imf, int16_t *cp, int16_t *icp); 


#endif
