#ifndef __LIN_FIT_H
#define __LIN_FIT_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define RP_BUF_SIZE 16384


void MakeDomain(int bufsize, float *ip);

int LinearFilter(float * domain, float * data, size_t n, float c0, float c1);

int gsl_fit_linear (const float *x, const float *y, const size_t n, float * c0, float * c1);

#endif	
