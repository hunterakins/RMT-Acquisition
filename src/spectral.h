#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int AutoPower(int16_t *dp, int16_t *idp, int16_t *power, int bufferSize);
		
int CrossPower(int16_t *dp, int16_t *idp, int16_t *dp1, int16_t *idp1, int16_t *cp, int16_t *icp, int bufferSize);
		
int Coherency(int bufferSize, int16_t *cp, int16_t *icp, int16_t * autopower, int16_t *autopower1, int16_t *coherency);
