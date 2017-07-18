/* Acquire a buffer of data for a given sample rate. A buffer is 16384 samples, of data type float. 
The data is then converted to doubles. (?)
The data is fitted to a line with the GNU scientific library function gsl_fit_linear.
The fit coefficients are passed into a function FirstOrderCorrect which updates the data buffer by filtering away the first order signal 
(This process is to eliminate DC offset and any low frequency (compared to the sample window) noise present)

I copied the FFT function (made a small modification)  from paulbourke.net/miscellaneous/dft, thanks Paul

I use a Hamming window to window my data. To generate the window values, I call GenWindow
These values are passed, along with the data, to Hadamard, a function which takes the (forgive my jargon) Hadamard product (https://en.wikipedia.org/wiki/Hadamard_product_(matrices)) of the window with my data and writes it into the data array. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <redpitaya/rp.h>
#include <unistd.h>
#include <math.h>
#include <gsl/gsl_fit.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <cblas.h>
#include <gsl/gsl_complex.h>



#define RP_BUF_SIZE 16384


void GenWindow(int bufsize, double *hp); /*generate hamming window vals */
void Hadamard(int bufsize, double *hp, double *dp); /* dot product the two arrays hp and dp */ 
void FFT(short int dir,long m,double *x,double *y); /* calculate the FFT */
int gsl_fit_linear(const double * x, const size_t xstride, const double * y, const size_t ystride, size_t n,
double * c0, double * c1, double * cov00, double * cov01, double * cov11, double
* sumsq); /* lin fit the data */
void MakeImagBuffer(int bufsize, double *ip); /* make buffer for imaginary component data since time series is totally real */
void MakeDomain(int bufsize, double *ip);
void FirstOrderCorrect(int bufsize, double c0, double c1, double *dp);
int GenWave(rp_channel_t channel, rp_waveform_t waveform, float amp, float freq, float offset); 
int PrintTime(struct timespec start);
int WriteFFTData(FILE *fd, double *dp, double *idp, int numvals);
int WriteData(FILE *fd, double *dp, int numvals);
int FloatToDouble(double *dp, float *tmpdp, int length);
int DebugDecimation(void);
int PrintVals(double *dp, int numvals);
int AutoPower(gsl_complex *spectrum, double *power, int bufferSize);
int MakeComplexArray(double *dp, double *idp, gsl_complex *spectrum, int bufsize);
double gsl_complex_abs(gsl_complex z);
int CrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, double *crosspower, int bufferSize);
gsl_complex gsl_complex_mul(gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_conjugate(gsl_complex z);	 



int main(int argc, char * argv[]) {
	int bufsize = RP_BUF_SIZE;
	double *idp = (double *)malloc(bufsize*sizeof(double)); /* buffer for imaginary time-domain data */
	double *dp = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	double *autopower = (double *)malloc(bufsize*sizeof(double));
	double *crosspower = (double *)malloc(bufsize*sizeof(double));
	double *idp2 = (double *)malloc(bufsize*sizeof(double)); /* buffer for imaginary time-domain data */
	double *dp2 = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp2 = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	double *autopower2 = (double *)malloc(bufsize*sizeof(double));
	gsl_complex *spectrum = (gsl_complex *)malloc(bufsize*sizeof(gsl_complex)/2);
	gsl_complex *spectrum2 = (gsl_complex *)malloc(bufsize*sizeof(gsl_complex)/2);
		
	// 180 ms for rp_Init 
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	// generate a wave to test the acquisition 	
	rp_waveform_t waveform = RP_WAVEFORM_SINE;
	rp_channel_t channel = RP_CH_1;
	float amplitude = 0.5;
	float freq = 15000;
	float offset = 0;	

	if (GenWave(channel, waveform, amplitude, freq, offset) != 0) {
		printf("yikes, error with wave generation");
	}


	uint32_t bufferSize = RP_BUF_SIZE;
	uint32_t *endplace;
	
	endplace = &bufferSize;
	printf("%u\n", bufferSize);	
	
	
	//rp_AcqReset();
	rp_acq_sampling_rate_t sampling_rate = RP_SMP_122_070K;	
	rp_AcqSetSamplingRate(sampling_rate);

	DebugDecimation();
	
	if (rp_AcqGetLatestDataV(channel, endplace, tempdp) != RP_OK) {
		printf("error with the acquisition");
	}
	
	rp_channel_t channelb = RP_CH_2;
	if (rp_AcqGetLatestDataV(channelb, endplace, tempdp2) != RP_OK) {
		printf("error with the acquisition");
	}
		
	/*sleep for the time it takes to write the samples */
	sleep(.1);

	FloatToDouble(dp, tempdp, bufferSize);
	FloatToDouble(dp2, tempdp2, bufferSize);
	FILE *input_data;
	FILE *input_data2;
	input_data = fopen("input_data", "w");
	input_data2 = fopen("input_data2", "w");
	
	WriteData(input_data, dp, bufferSize);
	WriteData(input_data2, dp2, bufferSize);
	// PrintVals(dp, bufferSize);

	/* get a domain for the linear fit: takes around 600 us */
	MakeDomain(bufsize, idp);
	

	// PrintVals(idp, bufferSize);	

	/* calculate c0 and c1 for the linear fit of [(0, dp0), (1, dp1), (2, dp2), ... ( bufsize, dpbufsize)]
	the values of 1 after idp and dp are to indicate the "stride", the spacing between adjacent array elements */ 
	/* 
	// takes 3.7 ms
	double c0;
	double c1;
	double cov00;
	double cov01;
	double cov11;
	double sumsq;
	gsl_fit_linear(idp, 1, dp, 1, (size_t) bufsize, &c0, &c1,  &cov00, &cov01, &cov11, &sumsq);
	printf("%lf\t%lf\t", c0, c1);	
	// get rid of DC and linear trends in the data
	// 
	FirstOrderCorrect(bufsize, c0, c1, dp);
	gsl_fit_linear(idp2, 1, dp2, 1, (size_t) bufsize, &c0, &c1,  &cov00, &cov01, &cov11, &sumsq);
	FirstOrderCorrect(bufsize, c0, c1, dp2);
	FILE *fitted_data; 
	fitted_data = fopen("fitted_data", "w");
	WriteData(fitted_data, dp, bufsize);	

	// write the window into idp, we use Hamming
	GenWindow(bufsize, idp);

	// window the data dp with the window function idp 	
	Hadamard(bufsize, dp, idp); 
	Hadamard(bufsize, dp2, idp);
	*/
	/* calculate fft and write to a file: takes on average less than 70 ms */
	short int dir = 1; /* direction of the fourier transform */ 
	long m = 14; /* number of samples in the time domain 2**14 = 16384 */
	FFT(dir, m, dp, idp); 
	FFT(dir, m, dp2, idp2); 
	
	FILE *fft; 
	fft = fopen("fft_data", "w");
	WriteFFTData(fft, dp, idp, bufsize/2);	

	FILE *fft2; 
	fft2 = fopen("fft_data2", "w");
	WriteFFTData(fft2, dp2, idp2, bufsize/2);	
	
	MakeComplexArray(dp, idp, spectrum, bufferSize/2);
	MakeComplexArray(dp2, idp2, spectrum2, bufferSize/2);
	AutoPower(spectrum, autopower, bufferSize/2);
	AutoPower(spectrum2, autopower2, bufferSize/2);
	
	FILE *fautopower;
	fautopower = fopen("autopower", "w");
	WriteData(fautopower, autopower, bufferSize/2);

	FILE *fautopower2;
	fautopower2 = fopen("autopower2", "w");
	WriteData(fautopower2, autopower2, bufferSize/2);	

	CrossPower(spectrum, spectrum2, crosspower,  bufferSize/2);
	FILE *crossp;
	crossp = fopen("crosspower", "w");
	WriteData(crossp, crosspower, bufferSize / 2);

	// free buffers
	free(dp);  
	free(idp); 
	free(tempdp);
	free(spectrum);	
	free(autopower);
	free(dp2);  
	free(idp2); 
	free(tempdp2);
	free(spectrum2);	
	free(autopower2);
	free(crosspower);
	// Print time elapsed since the clock_gettime(CLOCK_MONOTONIC, &start call) 	
	PrintTime(start);
	
	//release resources
	rp_Release();
	

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
	for (i = 0; i < bufferSize; i++) {
		*(power +i) = gsl_complex_abs(*(spectrum + i));
	}
	return 0;
}
		
int CrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, double *crosspower, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(crosspower + i) = GSL_REAL(gsl_complex_mul(*(spectrum + i), gsl_complex_conjugate(*(spectrum2 + i))));
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
	
	rp_GenReset();
	rp_GenAmp(channel, amp);
	

	rp_GenFreq(channel, freq);

	rp_GenWaveform(channel, waveform);
	
	if (rp_GenOutEnable(RP_CH_1) != RP_OK) {
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
	MakeImagBuffer((int) n, y);
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
