#ifndef __PROCESS_H
#define __PROCESS_H 

#include <stdio.h>
#include <math.h>
#include "window.h"
#include "lin_fit.h"
#include "spectral.h"
#include "fft.h"

typedef struct Data {
	size_t bufsize;
	float *dp;
	float *ip;
	float *domain;
	float *ap;
	float *rf;
	float *imf;
} Data;

void * Process(void * Data_Struct);


#endif
