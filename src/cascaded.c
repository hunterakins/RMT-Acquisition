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
	
	// ________________________intialize input buffers, some general variables____//
	bufsize = 16384;
	int_bufsize = 16384;
	uint_bufsize = 16384;
	float sampling_rates[6] = {125000000, 15625000, 1953000, 122070, 15628, 1907};
	float * dp = (void *) calloc(sizeof(float), bufsize);
	float * dp1 = (void *) calloc(sizeof(float), bufsize);

	// ________________________initialize conf file________________________________//
	config_t cfg;
	
	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, conf)) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}



	// ________________________look up cascade specific settings___________________//
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


	// ________________________look up display settings_____________________________//
	int freq_domain;
	config_lookup_bool(&cfg, "display.freq_domain", &freq_domain);
	int terminal;
	config_lookup_bool(&cfg, "display.terminal", &terminal);
	int write;
	config_lookup_bool(&cfg, "display.write", &write);
	const char * file_folder;
	if (write) {
		// where to write?
		config_lookup_string(&cfg, "write.file_folder", &file_folder);
		// place holder so that I can update the filenames for each channel
	}
	char name_holder[150];
	int plot;
	config_lookup_bool(&cfg, "display.plot", &plot);	
	// __________________________gnuplot initializations;
	char multiplot_base[40] = "set multiplot layout ";
	char multiplot_end[10] = " rowsfirst";
	// num of cols: 2 if odd, 3 if even
	char x[2];
	itoa((num_bands % 2) + 2, x);
	char y[2];
	itoa(num_bands / ((num_bands %2) + 2), y);
	char multiplot_init[10];
	char comma[2] = ",";
	strcpy(multiplot_init, y);
	strcat(multiplot_init, comma);
	strcat(multiplot_init, x);
	strcat(multiplot_base, multiplot_init);
	strcat(multiplot_base, multiplot_end);
	printf("Multiplot command: %s\n", multiplot_base);
	FILE *gnuplot = popen("gnuplot", "w");
	fprintf(gnuplot, "set term x11\n");
	if (num_bands > 1) {
		fprintf(gnuplot, "%s\n", multiplot_base);
	}
	// ________________________look up main settings ________________________________//


	//_________________________prepare for the first acquisition ____________________//

	float buffer_fill_time = 1000000*bufsize / sampling_rates[first_band];

	rp_channel_t channel = RP_CH_1;
	rp_channel_t channel1 = RP_CH_2;
	channel1 += channel;
	if (rp_Init() != RP_OK) {
		fprintf(stderr, "RP initialization failed");
		return 1;
	}
	rp_AcqReset();
	rp_AcqSetSamplingRate(first_band);	
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);
	rp_AcqSetTriggerDelay(0);
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
	rp_AcqStart();
	// let buffer fill
	usleep(buffer_fill_time);
	// determines depth of for loop 
	int final_index = acquire_time;
	// loop counter
	int i = 0;
	int j = 0;
	for (j=0;j<final_index; j++) {
		//________________wait for the trigger____________//
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				break;
			}
		}
		// acquire the data
		usleep(100);
		rp_AcqGetOldestDataV(channel, &uint_bufsize,  dp);
		int k;
		if (terminal) {
			printf("Channel 1: ");
			for (k =0; k < int_bufsize; k ++) {
				printf("%f\n", *(dp + k));
			}
		}
		else if (write) {
			// file name is loop number
			time_t curr_time;
			curr_time = time(NULL);
			struct tm * local_time;
			local_time = localtime(&curr_time);
			char ch1[40]; 
			strftime(ch1, 40, "%m%d%k%M%S", local_time);
			printf("%s\n", ch1);
			if (strcpy(name_holder, file_folder) == NULL) {
				fprintf(stderr, "issue with strcpy");
			}
			if (strcat(name_holder, ch1) == NULL) {
				fprintf(stderr, "issue with file naming"); 
			}
			printf("%s\n", name_holder);
			fflush(stdout);
			FILE * fd = fopen(name_holder, "w");
			WriteTimeData(fd, dp, bufsize);
			fclose(fd);
			printf("%s\n", name_holder);
			fflush(stdout);
		}
		else if (plot) {
			fprintf(gnuplot, "plot '-'\n");
			for (i = 0; i <int_bufsize; i++)
			    fprintf(gnuplot, "%f\n", dp[i]);
			fprintf(gnuplot, "e\n");
			fflush(gnuplot);
			// reset multiplot to clear old graphs 
			if ((num_bands > 1) & ((j % num_bands) == 0)) {
				fprintf(gnuplot, "unset multiplot\n");
				fprintf(gnuplot, "%s\n", multiplot_base);
			}
		}	
		rp_AcqStop();
		rp_AcqReset();
		// update sampling rate
		rp_AcqSetSamplingRate(first_band + j%num_bands);
		
		// update buffer_fill_time
		buffer_fill_time = (float) 1000000 * bufsize / (sampling_rates[first_band + j%num_bands]);
		
		printf("sampling rate: %f\n", sampling_rates[first_band + j%num_bands]);
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

	// _________________________________clean up _____________________________//
	config_destroy(&cfg);	
	fclose(gnuplot);
	free(dp);
	free(dp1);
	//gnuplot_cmd(h1, "unset multiplot");
	//gnuplot_close(h1);
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
