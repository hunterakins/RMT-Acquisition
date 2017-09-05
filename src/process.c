#include <stdio.h>
#include <math.h>
#include "lin_fit.h"
#include "spectral.h"
#include "fft.h"


// takes in the data struct, which contains the information for the channel and the allocated memory

void Process(void * Data_Struct) {

	// don't totally remember but think this has to do with subtleties of pthreads
	struct Data *d;

	d = (struct Data *) Data_Struct;

	size_t bufsize = (*d).buf_size;
	// declare array pointers	
	FILE *fd;
	int16_t *dp;
	int16_t *ip;
	int16_t *domain;
	int16_t *ap;
	int16_t *rf;
	int16_t *imf;
	
	// get array pointers from data struct
	dp = (*d).dp;
	dp = (*d).dp;
	domain = (*d).domain;
	ap = (*d).ap;
	rf = (*d).rf;
	imf = (*d).imf;

	MakeDomain(bufsize, domain);

	int16_t c0;
	int16_t c1;

	// do first order correction
	LinearFilter(domain, dp, bufsize, &c0, &c1);

	// fill domain with values of Hamming window 		
	GenWindow(bufsize, domain);

	// window dp with the vals in domain
	Hadamard(bufsize, dp, domain); 
		
	// copy
	memcpy(rf, dp, bufsize);
	memcpy(imf, ip, bufsize);

	short int dir = 1; /* direction of the fourier transform */ 
	long m = 14; /* number of samples in the time domain 2**14 = 16384 */

	FFT(dir, m, rf, imf); 
	printf("fft + %d\n", id); 	
	
	// compute ap (bufsize /2 because of purely real time domain data)
	
	AutoPower(rf, imf, ap, bufsize/2);

	
	return;
}
