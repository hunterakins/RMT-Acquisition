#ifndef __LIN_FIT_H
#define __LIN_FIT_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define RP_BUF_SIZE 16384


void MakeDomain(int bufsize, int16_t *ip);

int LinearFilter(int16_t * domain, int16_t * data, size_t n, int16_t c0, int16_t c1);

int gsl_fit_linear (const int16_t *x, const int16_t *y, const size_t n, int16_t * c0, int16_t * c1);

#endif	
