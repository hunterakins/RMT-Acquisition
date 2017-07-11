#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <redpitaya/rp.h>
#include <stdlib.h>
#include <unistd.h>

#define RP_BUF_SIZE 16384

int main(int argc, char *argv[]) {
	int i = 0;
	/*int samples = 1000; */
	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	
	rp_GenFreq(RP_CH_1, 100000.0);
	
	/* Generating amplitude */
	rp_GenAmp(RP_CH_1, 0.25);

	/* Generating wave form */
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);

	/* Enable channel */
	rp_GenOutEnable(RP_CH_1);



	float* data=(float *)malloc(RP_BUF_SIZE * sizeof(float));
	uint32_t bufferSize = RP_BUF_SIZE;
	uint32_t* endplace;
	endplace = &bufferSize;
	rp_AcqSetDecimation(RP_DEC_64); 
	rp_AcqSetSamplingRate(RP_SMP_1_953M);
	rp_AcqReset();
	rp_AcqGetLatestDataV(RP_CH_1, endplace, data);
	sleep(1);
	for (i = 0; i < bufferSize; i += 1) {
		fprintf(stdout, "%f\n", data[i]);
	}	
	/*
	for (i = 0; i < samples; i +=1) {
		rp_AcqReset();
		if (rp_AcqStart() != RP_OK) {
			printf("error with acquisition");
		}	
		rp_AcqGetLatestDataV(RP_CH_1, endplace, data);
		int j;
		sleep(.000131);
		for (j = 0; j < bufferSize; j += 1) {
			fprintf(fd, "%f\n", data[j]);
		}
	}
	*/
	free(data);
	rp_Release(); 	
}
