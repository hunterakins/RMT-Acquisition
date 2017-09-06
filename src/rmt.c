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

#define RP_BUF_SIZE 16384

int main(int argc, char * argv[]) {	
	size_t bufsize = RP_BUF_SIZE;
	int16_t * dp = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ip = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * domain = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ap = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * rf = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * imf = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * dp1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ip1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * domain1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * ap1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * rf1 = (void *) malloc(sizeof(int16_t) * bufsize);
	int16_t * imf1 = (void *) malloc(sizeof(int16_t) * bufsize);
	uint32_t size = bufsize;	


	rp_channel_t channel = RP_CH_1;

	rp_Init();

	sleep(1);

	rp_AcqStart();

	rp_AcqSetSamplingRate(RP_SMP_1_953M);
	
	rp_AcqGetLatestDataRaw(channel, &size, dp); 

	
	struct Data Channel1;		
	Channel1.dp = dp;
	
	int i;
	for(i=0;i<bufsize;i++) {
		printf("%" PRId16 "\n", *(Channel1.dp +i));	
	}

	Process(&Channel1);
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
