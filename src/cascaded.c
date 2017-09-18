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
#include "cascaded.h"
#include <libconfig.h>

#define RP_BUF_SIZE 16384

int cascade(void) {	
	config_t cfg;
	config_setting_t *setting;
	
	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, "../config/config.cfg")) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}


	setting = config_lookup(&cfg, "cascade");

	int acquire_time = config_setting_get_int_elem(setting, 0);
	// how many	
	int num_bands = config_setting_get_int_elem(setting, 1);		
	// which ones
	int first_band = config_setting_get_int_elem(setting, 2);

	if ((num_bands + first_band) > 6) {
		fprintf(stderr, "Number of bands plus first bands must be less than or equal to 6\n");
		return(EXIT_FAILURE);
	}

	
	config_destroy(&cfg);
	

	int16_t * dp = (void *) malloc(sizeof(int16_t) * bufsize);
	
	int16_t * dp1 = (void *) malloc(sizeof(int16_t) * bufsize);
	
	// place holder so that I can update the filenames for each channel
	//char name_holder[15] = "";
	*(dp + 0) = 0;	
	// initialize sampling rate to 1 
	rp_AcqSetSamplingRate(4);

	float buffer_fill_time = 1000000*BUFSIZE / sampling_rates[0];

	rp_channel_t channel = RP_CH_1;
	rp_channel_t channel1 = RP_CH_2;

	// just to avoid Werror when doing single channel acq
	channel1 += channel;
	
	// set up trigger stuff 
	rp_Init();

	rp_AcqReset();
	// 1 corresponds to 15.625MHz
	rp_AcqSetSamplingRate(1);
	
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);

	rp_AcqSetTriggerDelay(0);

        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	rp_AcqStart();
	
	// once I've began I need to wait buffer_fill_time for the samples to be written into adc buffer
	usleep(buffer_fill_time);
	// determines depth of for loop 
	int final_index = acquire_time;
	// i keeps track of which loop
	int i;
	int j;
	for (j=0;j<final_index; j++) {
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		usleep(800);
		rp_AcqGetOldestDataRaw(channel, &BUFSIZE, dp1);
			
		int k = 0;
		for (k =0; k < 100; k ++) {
			printf("%d\n", *(dp1 + k));
		}
		rp_AcqReset();
		// update sampling rate
		rp_AcqSetSamplingRate(first_band + j%num_bands);
		
		// update buffer_fill_time
		buffer_fill_time = 1000000 * BUFSIZE / (sampling_rates[first_band + j%num_bands]);
		
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

	rp_AcqStop();	
	return RP_OK;

}	
