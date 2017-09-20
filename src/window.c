#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// take hadamard (componentwise) product of two arrays, dp and hp, which will do a hamming window on dp
void Hadamard(int bufsize, float *dp, float *hp) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(dp + i) = *(dp + i) * *(hp + i);
	}
}

/* generate vals for the hamming window */
void GenWindow(int bufsize, float *hp) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(hp+i) = (float) .54 - .46*cos(2*M_PI*i/ bufsize);
	}
}

