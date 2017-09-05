#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQUARE(z) ((z)*(z))

// takes in spectrum with real values dp and im value idp
int AutoPower(int16_t *dp, int16_t *idp, int16_t *power, int bufferSize) {
	int i;
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
		
int Coherency(int bufferSize, int16_t *cp, int16_t *icp, int16_t * autopower, int16_t *autopower1, int16_t *coherency) {
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
