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
#include "gnuplot_i.h"

#define RP_BUF_SIZE 16384

int cascade(char conf[]) {	
	
	bufsize = 16384;
	float sampling_rates[6] = {125000000, 15625000, 1953000, 122070, 15628, 1907};

	config_t cfg;
	
	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, conf)) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}



	// look up cascade specific settings
	int acquire_time;
	config_lookup_int(&cfg, "cascade.acquire_time", &acquire_time);	
	printf("acquire time = : %d\n", acquire_time);
	int num_bands;
 	config_lookup_int(&cfg, "cascade.num_bands", &num_bands);		
	int first_band;
 	config_lookup_int(&cfg, "cascade.first_band", &first_band);		

	if ((num_bands + first_band) > 6) {
		fprintf(stderr, "Number of bands plus first bands must be less than or equal to 6\n");
		return(EXIT_FAILURE);
	}


	// look up display settings
	int terminal;
	config_lookup_bool(&cfg, "display.terminal", &terminal);
	int write;
	config_lookup_bool(&cfg, "display.write", &write);
	const char * file_folder;
	if (write) {
		// where to write?
		config_lookup_string(&cfg, "write.file_folder", &file_folder);
	}
	int plot;
	config_lookup_bool(&cfg, "display.plot", &plot);	
	int i = 0;
	gnuplot_ctrl * h1;
	h1 = gnuplot_init();
	//int16_t * dom = (void *) calloc(sizeof(int16_t), bufsize);
	double * dp = (void *) malloc(sizeof(double) * bufsize);
	// avoid Werror complaint
	*(dp + 0) = 0;	
	
	double * dp1 = (void *) malloc(sizeof(double) * bufsize);
	
	// place holder so that I can update the filenames for each channel
	char name_holder[30];
	 
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
	i = 0;
	int j = 0;
	for (j=0;j<final_index; j++) {
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		usleep(800);
		rp_AcqGetOldestDataRaw(channel, &bufsize, (int16_t *) dp1);
			
		int k = 0;
		if (terminal) {
			for (k =0; k < 100; k ++) {
				printf("%f\n", *(dp1 + k));
			}
		}
		else if (write) {
			// file name is loop number
			char file_name[10];
		 	itoa(i, file_name);
	
			if (strcpy(name_holder, file_folder) == NULL) {
				fprintf(stderr, "issue with strcpy");
			}
			if (strcat(name_holder, file_name) == NULL) {
				fprintf(stderr, "issue with file naming"); 
			}
			FILE * fd = fopen(name_holder, "w");
			WriteTimeData(fd, dp1, bufsize);
			fclose(fd);
		}
		else if (plot) {
			gnuplot_resetplot(h1);
			printf("ready to plot \n");
			gnuplot_setstyle(h1, "points");
			gnuplot_plot_x(h1, (double * ) dp1, bufsize, "nice");			
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
		printf("Trigger state: %d\n",state);
		// increment loop counter
		i += 1;
	}

	config_destroy(&cfg);	
	free(dp);
	free(dp1);
	gnuplot_close(h1);
	rp_AcqStop();	
	return RP_OK;

}	

 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
