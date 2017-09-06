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
	int16_t *dp;
	int16_t *ip;
	int16_t *domain;
	int16_t *ap;
	int16_t *rf;
	int16_t *imf;
} Data;

void Process(struct Data * Data_Struct);


#endif
