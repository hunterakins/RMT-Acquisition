#include <stdio.h>
#include <math.h>
#include <"lin_fit.h">
#include <"spectrum.h">
#include <"fft.h">


// takes in the data struct, which contains the information for the channel and the allocated memory

void Process(void * Data_Struct) {

	// don't totally remember but think this has to do with subtleties of pthreads
	struct Data *d;

	d = (struct Data *) Data_Struct;

	size_t bufsize = (*d).buf_size;


	// declare array pointers	
	int16_t * dp;
	int16_t * domain;
	int16_t * ip;
	int16_t * autopower;
	int16_t * realf;
	int16_t *imf;
	int16_t *wind;
	int id;
	
	// get array pointers from data struct
	tempdp = (*d).tempdp;
	idp = (*d).idp;
	dp = (*d).dp;
	autopower = (*d).autopower;
	id = (*d).id;
	realf = (*d).realf;
	imf = (*d).imf;
	wind = (*d).wind;

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
	memcpy(realf, dp, bufsize);
	memcpy(imf, ip, bufsize);

	short int dir = 1; /* direction of the fourier transform */ 
	long m = 14; /* number of samples in the time domain 2**14 = 16384 */

	FFT(dir, m, realf, imf); 
	printf("fft + %d\n", id); 	
		
	
	AutoPower(realf, imf, autopower, bufsize/2);

	FILE *fd;
	fd = (*d).fd;
	WriteAllData(fd, dp, wind, spectrum, autopower, bufsize);
	
	return NULL;
}
