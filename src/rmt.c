#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <gsl/gsl_fit.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <cblas.h>
#include <gsl/gsl_complex.h>
#include "/opt/redpitaya/include/redpitaya/rp.h"
#include <pthread.h>
#include <rmt.h>

#define RP_BUF_SIZE 16384



typedef struct Data {
	int buf_size;
	FILE *fd;
	double *idp;
	double *dp;
	float *tempdp;
	double *autopower;
	gsl_complex *crosspower;
	double *coherency;
	gsl_complex *spectrum;
	int id;
	double *realf;
	double *imf;
	double *wind;
} Data;

int WriteAllData(FILE * fd, double *inputdata, double * fit_and_window, gsl_complex * spectrum, double * autopower, int len) {
	int i = 0;
	for (i = 0; i < len / 2; i++) {
		fprintf(fd, "%lf\t%lf\t%lf\t%lf\t%lf\n", *(inputdata + 2*i), *(fit_and_window + 2*i), GSL_REAL(*(spectrum +i)), GSL_IMAG(*(spectrum +i)), *(autopower +i));
	}
	return 0;
}	

int GetRealArray(gsl_complex *array, double * real, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(real+i) = GSL_REAL(*(array+i));
	} 
	return 0;
}

int MakeComplexArray(double *dp, double *idp, gsl_complex *spectrum, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		GSL_SET_COMPLEX(spectrum + i, *(dp + i), *(idp+i));
	}
	return 0;
}

int AutoPower(gsl_complex *spectrum, double *power, int bufferSize) {
	int i;
	double pow;
	for (i = 0; i < bufferSize; i++) {
		pow = 1000000* gsl_complex_abs(*(spectrum + i));
		pow = pow * pow;
		*(power + i) = pow;
	}
	return 0;
}
		
int CrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, gsl_complex *crosspower, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(crosspower + i) = gsl_complex_mul(*(spectrum + i), gsl_complex_conjugate(*(spectrum2 + i)));
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
//write data to stdout for debugging
int PrintVals(double *dp, int numvals) {
	int i = 0;
	for (i = 0; i < numvals; i++) {
		printf("%lf\n", *(dp +i));
	}
	return 0;
}


// print out decimation to stdout
int DebugDecimation() {
	uint32_t decf;
	uint32_t *decfp;
	decfp = &decf;
	printf("decimation level=");
	rp_AcqGetDecimationFactor(decfp);
	printf("%u\n", decf);
	return 0;
}

//abstraction for writing numvals of data pointed to by dp and idp to file pointed to by fd 
int WriteFFTData(FILE *fd, double *dp, double *idp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%lf\t%lf\n", *(dp+i), *(idp+i));
	}
	return 0;
}

int WriteSpectralData(FILE *fd, double *autopower, double *autopower2, gsl_complex * crosspower, double *coherency, int numvals) {
	int i;
	for (i=0;i<numvals; i++) {
		fprintf(fd, "%lf\t%lf\t%lf\t%lf\t%lf\n", *(autopower + i), *(autopower2 + i), 1000000000000*GSL_REAL(*(crosspower + i)), 1000000000000*GSL_IMAG(*(crosspower + i)), *(coherency + i));
	}
	return 0;
}

int WriteData(FILE *fd, double *dp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%lf\n", *(dp+i));
	}
	return 0;
}

int FloatToDouble(double *dp, float *tempdp, int len) {
	int i;
	for (i = 0; i < len; i++) {
		*(dp + i) = (double) *(tempdp + i);
	}
	return 0;
}


//abstraction for the timer function. Start is declared at the beginning. This will compute the time elasped since then and print it to stdout
int PrintTime(struct timespec start){
	long elapsedtime;
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsedtime = end.tv_nsec - start.tv_nsec;
	printf("Time elapsed = ");
	printf("%lu\n", elapsedtime);
	return 0;
}


/* make a domain for the linear fit, simply the numbers 0, 1, ..., bufsize */

void MakeDomain(int bufsize, double *ip) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(ip + i) = (double) i;
	}
}

/* subtract away the first order fit to get rid of low frequency noise and DC offset 
line is of the form c0 + c1x
*/
void FirstOrderCorrect(int bufsize, double c0, double c1, double *dp) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		/* debug stuff 
		printf("original val");
		printf("%lf\n", *(dp + i));
		*/
		*(dp+i) = *(dp + i) - c0 - c1*i;
		/*
		printf("post fit val");
		printf("%lf\n", *(dp + i));
		*/
	}
}

void MakeImagBuffer(int bufsize, double *ip) {
	int i = 0;
	for (i = 0; i < bufsize; i ++) {
		*(ip+i) = 0;
	}
}

/* take in array * dp and array of hamming window values *hpand write the pairwise product of the elements and write it into *dp */

void Hadamard(int bufsize, double *dp, double *hp) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(dp + i) = *(dp + i) * *(hp + i);
	}
}

/* generate vals for the hamming window */
void GenWindow(int bufsize, double *hp) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(hp+i) = (double) .54 - .46*cos(2*M_PI*i/ bufsize);
	}
}


	
/* abstraction for generating a wave for testing */
int GenWave(rp_channel_t channel, rp_waveform_t waveform, float amp, float freq, float offset) {
	
	rp_GenAmp(channel, amp);
	

	rp_GenFreq(channel, freq);

	rp_GenWaveform(channel, waveform);
	
	if (rp_GenOutEnable(channel) != RP_OK) {
		printf("error with gen output");
		return -1;
	}	
	return 0;
}



/*
Paul's comment
This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/




void FFT(short int dir,long m,double *x, double *y) {
   long n,i,i1,j,k,i2,l,l1,l2;
   float c1,c2,tx,ty,t1,t2,u1,u2,z;
   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++) { 
      n *= 2;
   }
   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0; 
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0; 
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1; 
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1) 
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }
}


void * Process(void * Data_Struct) {
	struct Data *d;
	d = (struct Data *) Data_Struct;
	int bufferSize = (*d).buf_size;
	
	float * tempdp;
	double * idp;
	double * dp;
	double * autopower;
	double * realf;
	double *imf;
	double *wind;
	gsl_complex * spectrum;
	int id;
	
	// get arrays
	tempdp = (*d).tempdp;
	idp = (*d).idp;
	dp = (*d).dp;
	autopower = (*d).autopower;
	spectrum = (*d).spectrum;
	id = (*d).id;
	realf = (*d).realf;
	imf = (*d).imf;
	wind = (*d).wind;
	FloatToDouble((*d).dp, tempdp, bufferSize) ;
	MakeDomain(bufferSize, idp);

	/* calculate c0 and c1 for the linear fit of [(0, dp0), (1, dp1), (2, dp2), ... ( bufsize, dpbufsize)]
	the values of 1 after idp and dp are to indicate the "stride", the spacing between adjacent array elements */ 
	 
	// takes 3.7 ms
	double c0;
	double c1;
	double cov00;
	double cov01;
	double cov11;
	double sumsq;
	memcpy(wind, dp, bufferSize);
	gsl_fit_linear(idp, 1, dp, 1, (size_t) bufferSize, &c0, &c1,  &cov00, &cov01, &cov11, &sumsq);
	printf("fit + %d\n", id); 	
	FirstOrderCorrect(bufferSize, c0, c1, wind);
	printf("correct + %d\n", id); 	
	GenWindow(bufferSize, idp);

	printf("windows + %d\n", id); 	
	// window the data dp with the window function idp 	
	Hadamard(bufferSize, wind, idp); 
	
	
	printf("had + %d\n", id); 	

	memcpy(realf, wind, bufferSize);
	memcpy(imf, wind, bufferSize);

	short int dir = 1; /* direction of the fourier transform */ 
	long m = 14; /* number of samples in the time domain 2**14 = 16384 */
	FFT(dir, m, realf, imf); 
	printf("fft + %d\n", id); 	
		
	MakeComplexArray(realf, imf, spectrum, bufferSize/2);
	
	AutoPower(spectrum, autopower, bufferSize/2);

	FILE *fd;
	fd = (*d).fd;
	WriteAllData(fd, dp, wind, spectrum, autopower, bufferSize);
	
	return NULL;
}
