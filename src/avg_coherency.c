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
int AccAutoPower(gsl_complex *spectrum, double *power, int bufferSize);
int MakeComplexArray(double *dp, double *idp, gsl_complex *spectrum, int bufsize);
double gsl_complex_abs(gsl_complex z);
int AccCrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, gsl_complex *crosspower, int bufferSize);
gsl_complex gsl_complex_mul(gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_conjugate(gsl_complex z);	 
gsl_complex gsl_complex_add(gsl_complex z1, gsl_complex z2);
int Coherency(int bufferSize, gsl_complex *crosspower, double *autopower, double *autopower2, double *coherency);
int GetRealArray(gsl_complex *array, double * real, int bufferSize);
int WriteSpectralData(FILE *fd, double *autopower, double *autopower2, gsl_complex * crosspower, double *coherency, int numvals);
void * Process(void * datastruct);
int WriteAllData(FILE * fd, double *inputdata, double * fit_and_window, gsl_complex * spectrum, double * autopower, int len);


typedef struct Data {
	int buf_size;
	FILE *fd;
	double *idp;
	double *dp;
	float *tempdp;
	double *autopower;
	double *coherency;
	gsl_complex *spectrum;
	int id;
	double *realf;
	double *imf;
	double *wind;
} Data;



int main(int argc, char * argv[]) {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// 180 ms for rp_Init 
	if (rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	int bufsize = RP_BUF_SIZE;
	double *idp = (double *)malloc(bufsize*sizeof(double)); /* buffer for imaginary time-domain data */
	double *dp = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	double *realf = (double *)malloc(bufsize*sizeof(double)); //real part of FFT data
	double *imf = (double *)malloc(bufsize*sizeof(double)); // im part of FFT data
	double *wind = (double *)malloc(bufsize*sizeof(double)); //buff for window
	double *autopower = (double *)malloc(bufsize*sizeof(double)/2); //divide by 2 because nyquist limit, real input is symmetric about nyquist
	gsl_complex *crosspower = (gsl_complex *)malloc(bufsize*sizeof(gsl_complex)/2);
	double *idp2 = (double *)malloc(bufsize*sizeof(double)); /* buffer for imaginary time-domain data */
	double *dp2 = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp2 = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	double *autopower2 = (double *)malloc(bufsize*sizeof(double) / 2);
	double *realf2 = (double *)malloc(bufsize*sizeof(double)); //real part of FFT data
	double *imf2 = (double *)malloc(bufsize*sizeof(double)); // im part of FFT data
	double *coherency = (double *)malloc(bufsize*sizeof(double) / 2);
	double *wind2 = (double *)malloc(bufsize*sizeof(double));
	gsl_complex *spectrum = (gsl_complex *)malloc(bufsize*sizeof(gsl_complex)/2);
	gsl_complex *spectrum2 = (gsl_complex *)malloc(bufsize*sizeof(gsl_complex)/2);
	



	// generate a wave to test the acquisition 	
	
	//rp_waveform_t waveform = RP_WAVEFORM_SINE;
	rp_channel_t channel = RP_CH_1;
	/*
	float amplitude = 1;
	float freq = 30000;
	float offset = 0;	

	if (GenWave(channel, waveform, amplitude, freq, offset) != 0) {
		printf("yikes, error with wave generation");
	}
	*/
	printf("Init and gen wave time");
	PrintTime(start);

	clock_gettime(CLOCK_MONOTONIC, &start);

	uint32_t bufferSize = RP_BUF_SIZE;
	uint32_t *endplace;
	
	endplace = &bufferSize;
	printf("%u\n", bufferSize);	
	
	
	rp_AcqReset();
	//OPtions: 1_970K, 15_258K, 122_070K, 1_953M, 15_625M, 125M 
	rp_acq_sampling_rate_t sampling_rate = RP_SMP_122_070K;	
	rp_AcqSetSamplingRate(sampling_rate);

	DebugDecimation();
	rp_AcqStart();

	sleep(1);	//avoid synchronization error, not sure if it's from scp files that are being written or what...
		
	int  i = 0;
	for (i = 0; i < 10; i ++) {		
		printf("here/n");
		if (rp_AcqGetLatestDataV(channel, endplace, tempdp) != RP_OK) {
			printf("error with the acquisition");
		}
		rp_channel_t channelb = RP_CH_2;
		if (rp_AcqGetLatestDataV(channelb, endplace, tempdp2) != RP_OK) {
			printf("error with the acquisition");
		}
		
		struct Data Channel1;

		FILE *file1;
		int id1 = 1;

		file1 = fopen("channel1", "w");

		Channel1.buf_size = bufsize;
		Channel1.fd = file1;
		Channel1.idp = idp;
		Channel1.dp = dp;
		Channel1.tempdp = tempdp;
		Channel1.autopower = autopower;
		Channel1.spectrum = spectrum;
		Channel1.id = id1;
		Channel1.realf = realf;
		Channel1.imf = imf;
		Channel1.wind = wind;	
		struct Data Channel2;


		FILE *file2; 
		int id2 = 2;

		file2 = fopen("channel2", "w");

		Channel2.buf_size = bufsize;
		Channel2.fd = file2;
		Channel2.idp = idp2;
		Channel2.dp = dp2;
		Channel2.tempdp = tempdp2;
		Channel2.autopower = autopower2;
		Channel2.spectrum = spectrum2;
		Channel2.id = id2;
		Channel2.wind = wind2;
		Channel2.realf = realf2;
		Channel2.imf = imf2;

		pthread_t thread1;
		pthread_t thread2;
		printf("%lf\n", GSL_REAL(crosspower[0]));
		
		pthread_create(&thread1, NULL, Process, &Channel1);
		pthread_create(&thread2, NULL, Process, &Channel2);
		AccCrossPower(Channel1.spectrum, Channel2.spectrum, crosspower, bufferSize / 2);
		printf("%lf\n", GSL_REAL(crosspower[0]));
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);

		
		

	}
	Coherency(bufferSize / 2, crosspower, autopower, autopower2, coherency);
	FILE *avg_coh;
	avg_coh = fopen("avg_coh", "w");
	WriteData(avg_coh, coherency, bufferSize/2);
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
	free(coherency);
	free(realf);
	free(imf);
	free(realf2);
	free(imf2);
	free(wind);
	// Print time elapsed since the clock_gettime(CLOCK_MONOTONIC, &start call) 	
	printf("free my buffer\n");
	PrintTime(start);
	//release resources
	rp_Release();
	return 0;
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
	
	printf("here");
	AccAutoPower(spectrum, autopower, bufferSize/2);
		
	FILE *fd;
	fd = (*d).fd;
	WriteAllData(fd, dp, wind, spectrum, autopower, bufferSize);
	
	return NULL;
}

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

int AccAutoPower(gsl_complex *spectrum, double *power, int bufferSize) {
	int i;
	double pow;
	for (i = 0; i < bufferSize; i++) {
		pow = 1000000* gsl_complex_abs(*(spectrum + i));
		pow = pow * pow;
		*(power + i) = *(power + i) + pow;
	}
	return 0;
}
		
int AccCrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, gsl_complex *crosspower, int bufferSize) {
	int i;
	for (i = 0; i < bufferSize; i++) {
		*(crosspower + i) = gsl_complex_add(*(crosspower + i), gsl_complex_mul(*(spectrum + i), gsl_complex_conjugate(*(spectrum2 + i))));
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
