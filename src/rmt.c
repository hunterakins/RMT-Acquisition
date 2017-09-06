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
	uint32_t size = bufsize;	


	rp_channel_t channel = RP_CH_1;

	rp_Init();

	sleep(1);

	rp_AcqStart();

	rp_AcqGetLatestDataRaw(channel, &size, dp); 

	
	struct Data Channel1;		
	Channel1.dp = dp;
	
	int i;
	for(i=0;i<bufsize;i++) {
		printf("%" PRId16 "\n", *(Channel1.dp +i));	
	}
	free(dp);

	rp_Release();
	return 0;

}	
