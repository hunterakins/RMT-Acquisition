#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQUARE(z) ((z)*(z))

// takes in spectrum with real values dp and im value idp
int AutoPower(float *dp, float *idp, float *power, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(power + i) = 1000000*SQUARE(*(dp+i)) + SQUARE(*(idp+i)) ;
	}
	return 0;
}
		
// idp
int CrossPower(float *dp, float *idp, float *dp1, float *idp1, float *cp, float *icp, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(cp + i) = 100000 * ((*(dp+i)) * (*(dp1)) + (*(idp+i)) * (*(idp1 +i))); 
		*(icp + i) = 100000*((*(idp+i)) * (*(dp1+i)) - (*(dp + i)) * (*(idp1 + i))); 
	}
	return 0;
}
		
int Coherency(int bufferSize, float *cp, float *icp, float * autopower, float *autopower1, float *coherency) {
	int i;
	double num;
	double denom;
	for (i = 0; i < bufferSize; i++) {
		num = SQUARE(*(cp + i)) + SQUARE(*(icp + i)) ;
		denom = *(autopower + i) * *(autopower1 + i);
		if (denom == (double) 0) {
			*(coherency + i) = 0;
		}
		else {
			*(coherency + i) = num / denom;
		} 
	}
	return 0;

}
