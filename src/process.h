#include <stdio.h>
#include <math.h>
#include <"lin_fit.h">
#include <"spectrum.h">
#include <"fft.h">

typedef struct Data {
	size_t bufsize;
	FILE *fd;
	int16_t *dp;
	int16_t *ip;
	int16_t *domain;
	int16_t *ap;
	int16_t *rf;
	int16_t *imf;
} Data;

void Process(void * Data_Struct);



