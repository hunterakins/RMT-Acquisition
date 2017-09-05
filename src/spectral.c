#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQUARE(z) = ((z)*(z))

// takes in spectrum with real values dp and im value idp
int AutoPower(int16_t *dp, int16_t *idp, double *power, int bufferSize) {
	int i;
	int16_t pow;
	for (i = 0; i < bufferSize; i++) {
		*(power + i) = SQUARE(*(dp+i)) + SQUARE(*(idp+i)) ;
	}
	return 0;
}
		
// idp
int CrossPower(int16_t *dp, int16_t *idp, int16_t *dp1, int16_t *idp1, int16_t *cp, int16_t *icp, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(cp + i) = (*(dp+i)) * (*(dp1)) + (*(idp+i)) * (*(idp1 +i)); 
		*(icp + i) = (*(idp+i)) * (*(dp1+i)) - (*(dp + i)) * (*(idp1 + i)); 
	}
	return 0;
}
		
int Coherency(int bufferSize, gsl_complex *crosspower, double *autopower, double *autopower2, double *coherency) {
	int i;
	double num;
	double denom;
	for (i = 0; i < bufferSize; i++) {
		num = 1000000*gsl_complex_abs(*(crosspower + i));
		num = 1000000000000* num * num;
		denom = *(autopower + i) * *(autopower2 + i);
		if (denom == (double) 0) {
			*(coherency + i) = 0;
		}
		else {
			*(coherency + i) = num / denom;
		} 
	}
	return 0;

}
