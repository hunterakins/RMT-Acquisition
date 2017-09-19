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

int cascade(char conf[]) {	
	bufsize = 16384;
	float sampling_rates[6] = {125000000, 15625000, 1953000, 122070, 15628, 1907};
	FILE *fd;
	char conf_base[] = "/home/redpitaya/RedPitaya/RMT-Acquisition/config/\0";
	if (strcat(conf_base, conf) == NULL ) {
		fprintf(stderr, "string concat/ failed on config file\n");
	}
		
	fd = fopen(conf_base, "r");
	if (fd == NULL) {
		fprintf(stderr, "can't open file");
		return EXIT_FAILURE;
	}
	config_t cfg;
	config_setting_t *setting;
	
	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read(&cfg, fd)) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}

	printf("0\n");

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

	printf("1\n");	
	config_destroy(&cfg);	

	int16_t * dp = (void *) malloc(sizeof(int16_t) * bufsize);
	
	int16_t * dp1 = (void *) malloc(sizeof(int16_t) * bufsize);
	// place holder so that I can update the filenames for each channel
	//char name_holder[15] = "";
	*(dp + 0) = 0;	
	// initialize sampling rate to 1 
	printf("2\n");	

	float buffer_fill_time = 1000000*bufsize / sampling_rates[first_band];

	rp_channel_t channel = RP_CH_1;
	rp_channel_t channel1 = RP_CH_2;

	// just to avoid Werror when doing single channel acq
	channel1 += channel;
	
	// set up trigger stuff 
	if (rp_Init() != RP_OK) {
		fprintf(stderr, "RP initialization failed");
		return 1;
	}

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
	printf("3\n");
	for (j=0;j<final_index; j++) {
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		usleep(800);
		rp_AcqGetOldestDataRaw(channel, &bufsize, dp1);
			
		int k = 0;
		for (k =0; k < 100; k ++) {
			printf("%d\n", *(dp1 + k));
		}
		rp_AcqReset();
		// update sampling rate
		rp_AcqSetSamplingRate(first_band + j%num_bands);
		
		// update buffer_fill_time
		buffer_fill_time = 1000000 * bufsize / (sampling_rates[first_band + j%num_bands]);
		
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
	free(dp);
	free(dp1);
	fclose(fd);
	rp_AcqStop();	
	return RP_OK;

}	
