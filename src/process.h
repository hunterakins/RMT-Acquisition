#include <stdio.h>
#include <math.h>
#include <"lin_fit.h">
#include <"spectrum.h">
#include <"fft.h">

typedef struct Data {
	int bufsize;
	FILE *fd;
	double *idp;
	double *dp;
	float *tempdp;
	double *ap;
	int16_t *cp;
	int16_t *icp;
	int16_t *coh;
	double *rf;
	double *imf;
} Data;

void Process(void * Data_Struct);



