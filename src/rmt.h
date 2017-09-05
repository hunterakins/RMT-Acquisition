#include <stdio.h>
#include <stdlib.h>

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


