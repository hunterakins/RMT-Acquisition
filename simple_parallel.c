/*
Parallized version of acquire_and_process

Set up pipeline for reading and writing of data
:q


 Acquire a buffer of data for a given sample rate. A buffer is 16384 samples, of data type float. 
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


int GenWave(rp_channel_t channel, rp_waveform_t waveform, float amp, float freq, float offset); 
int PrintTime(struct timespec start);
int WriteData(FILE *fd, double *dp, int numvals);
void * FloatToDouble(void * dtf);
int DebugDecimation(void);


/* struct to pass to pthread */
typedef struct float_double {
	double * double_points;
	float * float_points;
	int len;
	int id;
} float_double;

int main(int argc, char * argv[]) {
	// 180 ms for rp_Init 
	if (rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	int bufsize = RP_BUF_SIZE;
	double *dp = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	double *dp2 = (double *)malloc(bufsize*sizeof(double)); /* buffer for real time-domain data */
	float *tempdp2 = (float *)malloc(bufsize*sizeof(float)); /* buffer to hold the floats read into the adc buffer */
	
		
	
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	// generate a wave to test the acquisition 	
	rp_waveform_t waveform = RP_WAVEFORM_SINE;
	rp_channel_t channel = RP_CH_1;
	float amplitude = 1;
	float freq = 30000;
	float offset = 0;	

	if (GenWave(channel, waveform, amplitude, freq, offset) != 0) {
		printf("yikes, error with wave generation");
	}

	printf("Gen wave time");
	PrintTime(start);

	clock_gettime(CLOCK_MONOTONIC, &start);

	uint32_t bufferSize = RP_BUF_SIZE;
	uint32_t *endplace;
	
	endplace = &bufferSize;
	printf("%u\n", bufferSize);	
	
	
	rp_AcqReset();
	//OPtions: 1_970K, 15_258K, 122_070K, 1_953M, 15_625M, 125M 
	rp_acq_sampling_rate_t sampling_rate = RP_SMP_1_953M;	
	rp_AcqSetSamplingRate(sampling_rate);

	DebugDecimation();
	rp_AcqStart();
	
	sleep(.1);
		
	if (rp_AcqGetLatestDataV(channel, endplace, tempdp) != RP_OK) {
		printf("error with the acquisition");
	}
	
	sleep(.1);
	rp_channel_t channelb = RP_CH_2;
	if (rp_AcqGetLatestDataV(channelb, endplace, tempdp2) != RP_OK) {
		printf("error with the acquisition");
	}
	printf("acq start time");	
	PrintTime(start);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	
	/*sleep for the time it takes to write the samples */
	sleep(.1);
	printf("sleeptime");
	PrintTime(start);
	
	clock_gettime(CLOCK_MONOTONIC, &start);




	
	struct float_double dtf;
	dtf.double_points = dp;
	dtf.float_points = tempdp;
	dtf.len = bufsize;
	dtf.id = 1;	

	struct float_double dtf2;
	dtf2.double_points = dp2;
	dtf2.float_points = tempdp2;
	dtf2.len = bufsize;
	dtf.id = 2;
	
	pthread_t thread1;
	pthread_t thread2; 		
	pthread_create(&thread1, NULL, (void *) FloatToDouble, (void *) &dtf);	
	pthread_create(&thread2, NULL, (void *) FloatToDouble, (void *) &dtf2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	printf("float to double");	
	PrintTime(start);
	
	clock_gettime(CLOCK_MONOTONIC, &start);
	PrintTime(start);
	
	clock_gettime(CLOCK_MONOTONIC, &start);

	// free buffers
	free(dp);  
	free(tempdp);
	free(dp2);  
	free(tempdp2);
	// Print time elapsed since the clock_gettime(CLOCK_MONOTONIC, &start call) 	
	printf("free my buffer\n");
	PrintTime(start);
	
	//release resources
	rp_Release();
	

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

int WriteData(FILE *fd, double *dp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%lf\n", *(dp+i));
	}
	return 0;
}

void * FloatToDouble(void *float_double_struct) {
	int i;
	struct float_double *dtf;
	dtf = (struct float_double *) float_double_struct;
	int len = (*dtf).len;
	for (i = 0; i < len; i++) {
		*((*dtf).double_points + i) = (double) *((*dtf).float_points + i);
	}
	
	return NULL;
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
