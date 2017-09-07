#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "/opt/redpitaya/include/redpitaya/rp.h"
#include <pthread.h>
#include "fft.h"
#include "lin_fit.h"
#include "write.h"
#include "spectral.h"
#include "window.h"
#include "process.h"
#include "rmt.h"
#include "inttypes.h"
#include <time.h>
#include <string.h>

#define RP_BUF_SIZE 16384

int main(int argc, char * argv[]) {	

	float sampling_rate = atof(*(argv+1));
	size_t bufsize = RP_BUF_SIZE;

	int16_t * dp = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ip = calloc(bufsize, sizeof(int16_t));
	int16_t * domain = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ap = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * rf = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * imf = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * dp1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ip1 = calloc(bufsize, sizeof(int16_t));
	int16_t * domain1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ap1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * rf1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * imf1 = (void *) malloc(sizeof(int16_t) * bufsize);
	uint32_t size = bufsize;	
	float buffer_fill_time = bufsize / sampling_rate;

	//time for the buffer to fill up in microseconds
	buffer_fill_time = buffer_fill_time * 1000000; 
	
	rp_channel_t channel = RP_CH_1;
	rp_channel_t channel1 = RP_CH_2;
	// just to avoid Werror
	channel1 += channel;
	
	// set up trigger stuff 
	rp_Init();

	rp_AcqReset();
	rp_AcqSetSamplingRate(RP_SMP_122_070K);
	
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);

	rp_AcqSetTriggerDelay(100);

        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	rp_AcqStart();
	
	// once I've began I need to wait buffer_fill_time for the samples to be written into adc buffer
	usleep(buffer_fill_time);
	// wait for trigger
	int j = 0;
	for (j=0;j<5;j++) {
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		printf("%d\n", j);
		
		// pre trigger valid points?
		uint32_t value;
		rp_AcqGetPreTriggerCounter(&value);
		printf("pretrigg counter: ");
		printf("%d\n", value);
			
		usleep(800);

		rp_AcqGetOldestDataRaw(channel, &size, dp1);
			
		int i = 0;	
		for (i=0;i<bufsize;i++) {
			printf("%" PRId16 "\n", *(dp1+i));
		}
		rp_AcqReset();
		rp_AcqSetSamplingRate(RP_SMP_122_070K);
		
		rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);

		rp_AcqSetTriggerDelay(100);


		rp_AcqStart();
		usleep(buffer_fill_time);
		rp_AcqGetTriggerState(&state);
		printf("Trigger statae: %d\n", state);
	}
	/* write	
	
	fd = fopen(asctime(timeinfo), "w");

	for(i=0;i<bufsize;i++) {
		fprintf(fd, "%" PRId16 "\t%" PRId16 "\n" , *(dp+i), *(dp1+i));
	}	
		


	*/

	struct Data Channel1;		
	Channel1.bufsize = bufsize;	
	Channel1.dp = dp;
	Channel1.ip = ip;
	Channel1.domain = domain;
	Channel1.ap = ap;
	Channel1.rf = rf;
	Channel1.imf = imf;
		
	struct Data Channel2;
	Channel2.bufsize = bufsize;
	Channel2.dp = dp1;
	Channel2.ip = ip1;
	Channel2.domain = domain1;
	Channel2.ap = ap1;
	Channel2.rf = rf1;
	Channel2.imf = imf1;
	

	Process(&Channel1);
	Process(&Channel2);

	pthread_t thread1;
	pthread_t thread2;
	
	pthread_create(&thread1, NULL, Process, &Channel1);
	pthread_create(&thread2, NULL, Process, &Channel2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	

	rp_AcqStop();

	free(dp);
	free(ip);
	free(domain);
	free(ap);
	free(rf);
	free(imf);
	free(dp1);
	free(ip1);
	free(domain1);
	free(ap1);
	free(rf1);
	free(imf1);

	rp_Release();
	return 0;

}	
