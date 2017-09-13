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

int main(int * acquire_time, char * filename) {	

	// size_t version of RP_BUF_SIZE for some rp functions
	BUFSIZE = RP_BUF_SIZE;
	// get the initial buffer_fill_time
	buffer_fill_time = 1000000 * BUFSIZE / (SAMPLING_RATES[0]);
	// place holder so that I can update the filenames for each channel
	char name_holder[15] = "";
	
	// initialize sampling rate to 1 
	rp_AcqSetSamplingRate(4);

	// determines depth of for loop 
	int final_index = * acquire_time;
	// i keeps track of which loop
	int i;
	for (j=0;j<final_index; j++) {
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		usleep(800);
		rp_AcqGetOldestDataRaw(channel, &size, dp1);
			
		int k = 0;
		for (k =0; k < 100; k ++) {
			printf("%d\n", *(dp1 + k));
		}
		rp_AcqReset();
		// update sampling rate
		rp_AcqSetSamplingRate(j%4 +1);
		
		// update buffer_fill_time
		buffer_fill_time = 1000000 * BUFSIZE / (SAMPLING_RATES[j%4]);
		
		// debug message
		printf("buffer_fill_time: %f\n", buffer_fill_time);
		
		// start filling the ADC buffer
		rp_AcqStart();
			
		// let it fill
		usleep(buffer_fill_time);
		
		// reset trigger stuff (seems to be necessary)
		rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);

		rp_AcqSetTriggerDelay(0);

		rp_AcqGetTriggerState(&state);
		//debug
		printf("Trigger statae: %d\n", state);
		//. increment loop counter
		i += 1;
	}
	
	return 0;

}	
