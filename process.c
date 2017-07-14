/*
Hunter's Note: copied the FFT script from paulbourke.net/miscellaneous/dft, thanks Paul

The process is a test for my red pitaya program. 

I will create fake data, 16384 "samples" that I generate externally. 

I will call this data "data" and open it with fopen

Then I will test my linfit functions and my windowing, as well as my FFT

*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<gsl/gsl_fit.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<cblas.h>


void GenWindow(int bufsize, double *hp); /*generate hamming window vals */
void Dot(int bufsize, double *hp, double *dp); /* dot product the two arrays hp and dp */ 
void FFT(short int dir,long m,double *x,double *y); /* calculate the FFT */
int gsl_fit_linear(const double * x, const size_t xstride, const double * y, const size_t ystride, size_t n,
double * c0, double * c1, double * cov00, double * cov01, double * cov11, double
* sumsq); /* lin fit the data */
void MakeImagBuffer(int bufsize, double *ip); /* make buffer for imaginary component data since time series is totally real */
void MakeDomain(int bufsize, double *ip);
void FirstOrderCorrect(int bufsize, double c0, double c1, double *dp);


void main(int argc, char * argv[]) {
	int i= 0;
	int j = 0;
	int bufsize = 16384;
	float freq = 100; /* 100 hertz sine wave */
	float spacing =  .001; /* 1kHz sample rate has a spacing of .001 seconds */
	double *idp = (double *)malloc(bufsize*sizeof(double)); /* buffer for imaginary time-domain data */
	double *dp = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	double *hp = (double *)malloc(bufsize*sizeof(double));  /* array of values for the Hamming window */
	short int dir = 1; /* direction of the fourier transform */
	long m = 14; /* number of samples in the time domain 2**14 = 16384 */
	FILE *fd;	/* file to get mock data from...for the red pitaya I won't need this */
	FILE *fftfd; /* file to write output to for verification and visualization */
	size_t fread_out; 
	size_t floatsize;
	char s[100];
	char *sp;
	sp = s;
	struct timespec start;
	struct timespec end;
	struct timespec elapsed;
	void *tzp;
	double c0;
	double c1;
	double cov00;
	double cov01;
	double cov11;
	double sumsq;
	long elapsedtime;
	int gsl;
	FILE *fitteddata;
	FILE *inputdata;
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	
	fd = fopen("data", "r");
	inputdata = fopen("input", "w");
	fftfd = fopen("fftdata", "w");
	fitteddata = fopen("fitteddata", "w");
	/*
	for (i = 0; i < bufsize; i++) {
		*(dom+i) = i;
	}
	*/
	/* read data from fd into dp */
	i = 0;
	while(fgets(sp, 99, fd) != '\0') {
		*(dp+i) = (double) atof(s);
		i++;
	}

	/* check dp values for debugging  
	for (i = 0; i < bufsize; i++) {
		printf("%lf\n", *(dp+i));
	}
	*/

	/*	write input data to file to test input	
	for (i = 0; i < bufsize; i++) {
		fprintf(inputdata, "%f\n", *(dp+i));
	}
	*/	
	/* get a domain to use in the linear fit */
	MakeDomain(bufsize, idp);
	/* check idp values for debugging   
	for (i = 0; i < bufsize; i++) {
		printf("%lf\n", *(idp+i));
	}
	*/	

	/* calculate c0 and c1 for the linear fit of [(0, dp0), (1, dp1), (2, dp2), ... ( bufsize, dpbufsize)]
	the values of 1 after idp and dp are to indicate the "stride", the spacing between adjacent array elements 
	  */

	gsl = gsl_fit_linear(idp, 1, dp, 1, (size_t) bufsize, &c0, &c1,  &cov00, &cov01, &cov11, &sumsq);	

	FirstOrderCorrect(bufsize, c0, c1, dp);
	/*
	for (i = 0; i < bufsize; i++) {
		fprintf(fitteddata, "%f\n", *(dp+i));
	}	
	*/
	/* fill idp with hamming window vals */
	GenWindow(bufsize, idp);
	
	/* check hamming values for debugging
	for (i = 0; i < bufsize; i++) {
		printf("%lf\n", *(idp+i));
	}
	*/

	/*dot dp with idp , which has the hamming values */  
	Dot(bufsize, dp, idp); 

	/* fill up a buffer with zeros */
	MakeImagBuffer(bufsize, idp); 
	

	FFT(dir, m, dp, idp); 
	/*
	for (i = 0; i < bufsize/2; i++) {
		fprintf(fftfd, "%f\n", *(dp+i));
	}
	*/	
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	elapsedtime = end.tv_nsec - start.tv_nsec;
	printf("%lu\n", elapsedtime);
	free(dp);  
	free(idp); 
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

/* take in array * dp and array of hamming window values and dot product them */

void Dot(int bufsize, double *dp, double *hp) {
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


	




/*
Paul's comment
This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/




void FFT(short int dir,long m,double *x,double *y) {
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
