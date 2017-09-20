#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int AutoPower(float *dp, float *idp, float *power, int bufferSize);
		
int CrossPower(float *dp, float *idp, float *dp1, float *idp1, float *cp, float *icp, int bufferSize);
		
int Coherency(int bufferSize, float *cp, float *icp, float * autopower, float *autopower1, float *coherency);
