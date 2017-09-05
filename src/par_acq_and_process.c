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
#include <rmt.h>

#define RP_BUF_SIZE 16384

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
	rp_waveform_t waveform = RP_WAVEFORM_SINE;
	rp_channel_t channel = RP_CH_1;
	float amplitude = 1;
	float freq = 30000;
	float offset = 0;	

	if (GenWave(channel, waveform, amplitude, freq, offset) != 0) {
		printf("yikes, error with wave generation");
	}

	printf("Init and gen wave time");
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

	sleep(1);	//avoid synchronization error, not sure if it's from scp files that are being written or what...
		
	if (rp_AcqGetLatestDataV(channel, endplace, tempdp) != RP_OK) {
		printf("error with the acquisition");
	}
	rp_channel_t channelb = RP_CH_2;
	if (rp_AcqGetLatestDataV(channelb, endplace, tempdp2) != RP_OK) {
		printf("error with the acquisition");
	}
	
	Data Channel1;

	FILE *file1;
	int id1 = 1;

	file1 = fopen("Data/channel1", "w");

	Channel1.buf_size = bufsize;
	Channel1.fd = file1;
	Channel1.idp = idp;
	Channel1.dp = dp;
	Channel1.tempdp = tempdp;
	Channel1.autopower = autopower;
	Channel1.crosspower = crosspower;
	Channel1.spectrum = spectrum;
	Channel1.id = id1;
	Channel1.realf = realf;
	Channel1.imf = imf;
	Channel1.wind = wind;	

	Data Channel2;



	FILE *file2; 
	int id2 = 2;

	file2 = fopen("Data/channel2", "w");

	Channel2.buf_size = bufsize;
	Channel2.fd = file2;
	Channel2.idp = idp2;
	Channel2.dp = dp2;
	Channel2.tempdp = tempdp2;
	Channel2.autopower = autopower2;
	Channel2.crosspower = crosspower;
	Channel2.spectrum = spectrum2;
	Channel2.id = id2;
	Channel2.wind = wind2;
	Channel2.realf = realf2;
	Channel2.imf = imf2;

	pthread_t thread1;
	pthread_t thread2;
	
	pthread_create(&thread1, NULL, Process, &Channel1);
	pthread_create(&thread2, NULL, Process, &Channel2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	
	CrossPower(spectrum, spectrum2, crosspower, bufferSize/2);

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



