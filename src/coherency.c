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

int coherency(char conf[]) {	
	
	// ________________________intialize input buffers, some general variables____//

	bufsize = 16384;
	int_bufsize = 16384;
	uint_bufsize = 16384;
	float sampling_rates[6] = {125000000, 15625000, 1953000, 122070, 15628, 1907};
	float * dp = (void *) calloc(bufsize, sizeof(float));
	float * ip = calloc(bufsize, sizeof(float));
	float * domain = (void *) calloc(bufsize, sizeof(float));
	float * ap = (void *) malloc(sizeof(float) * bufsize/2);
	float * rf = (void *) malloc(sizeof(float) * bufsize);
	float * imf = (void *) malloc(sizeof(float) * bufsize);
	float * dp1 = (void *) malloc(sizeof(float) * bufsize);
	float * ip1 = calloc(bufsize, sizeof(float));
	float * domain1 = (void *) malloc(sizeof(float) * bufsize);
	float * ap1 = (void *) malloc(sizeof(float) * bufsize/2);
	float * rf1 = (void *) malloc(sizeof(float) * bufsize);
	float * imf1 = (void *) malloc(sizeof(float) * bufsize);
	float * cohp = (void *) malloc(sizeof(float) * bufsize);
	float * cp = (void *) malloc(sizeof(float) * bufsize);
	float * icp = (void *) malloc(sizeof(float) * bufsize);

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

	// __________________________gnuplot initializations_____________________________//

	char multiplot_base[40] = "set multiplot layout ";
	char multiplot_end[12] = " rowsfirst";
	// num of cols: 2 if odd, 3 if even
	char x[2];
	itoa(2*((num_bands % 2) + 2), x);
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


	// ______________________acquisition loop__________________//

	uint32_t trigger_pos;
	for (j=0;j<final_index; j++) {
		trigger_pos = 0;
		//________________wait for the trigger____________//
		while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
				rp_AcqStop();
				break;
			}
		}
		// it appears i need to sleep here to allow the circular buffer to arrange itself ?
		usleep(buffer_fill_time);
		rp_AcqGetWritePointerAtTrig(&trigger_pos);
		//uint32_t  req_buf_size;
		printf("trigger_pos - bufsize: %d \n", bufsize - trigger_pos);
		// acquire the data
		rp_AcqGetOldestDataV(channel, &uint_bufsize,  dp);
		rp_AcqGetOldestDataV(channel1, &uint_bufsize,  dp1);
		// start filling the ADC buffer
		rp_AcqStart();
		int k;
		// run process on my two threads...
		pthread_create(&thread1, NULL, Process, &Channel1);
		pthread_create(&thread2, NULL, Process, &Channel2);

		
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);

		CrossPower(rf, imf, rf1, imf1, cp, icp, int_bufsize /2 );	
		Coherency(int_bufsize/2, cp, icp, ap, ap1, cohp);
		
		if (terminal) {
			printf("Channel 1 \t Channel 2\n");
			printf("_________________________\n");
			for (k =0; k < int_bufsize; k ++) {
				printf("%f\t%f\n", *(rf+ k), *(rf1 + k));
			}
		}
		else if (write) {
			// file name is loop number
			char file_name[30];
			
			time_t curr_time;
			curr_time = time(NULL);
			struct tm * local_time;
			local_time = localtime(&curr_time);
			strftime(file_name, 40, "%m%d%k%M%S", local_time);
			if (strcpy(name_holder, file_folder) == NULL) {
				fprintf(stderr, "issue with strcpy");
			}
			if (strcat(name_holder, file_name) == NULL) {
				fprintf(stderr, "issue with file naming"); 
			}
			FILE * fd = fopen(name_holder, "w");
			WriteTwoChannels(fd, dp, dp1, bufsize);
			fclose(fd);
			printf("%s\n", name_holder);
			fflush(stdout);
		}
		else if (plot) {
			fprintf(gnuplot, "plot '-'\n");
			for (i = 0; i < int_bufsize/2; i++) {
			    fprintf(gnuplot, "%f \n", ap[i]);
			}
			fprintf(gnuplot, "e\n");
			fflush(gnuplot);		
			fprintf(gnuplot, "plot '-'\n");
			for (i = 0; i < int_bufsize/2; i++) {
			    fprintf(gnuplot, "%f \n", ap1[i]);
			}
			fprintf(gnuplot, "e\n");
			fflush(gnuplot);
			
			fprintf(gnuplot, "plot '-'\n");
			for (i = 0; i < int_bufsize/2; i++) {
			    fprintf(gnuplot, "%f \n", ap1[i]);
			}
			fprintf(gnuplot, "e\n");
			fflush(gnuplot);
			// reset multiplot to clear old graphs 
			if ((num_bands > 1) & ((j % num_bands) == 0)) {
				fprintf(gnuplot, "unset multiplot\n");
				fprintf(gnuplot, "%s\n", multiplot_base);
			}
		}	

		// ___________________________prepare for trigger and next round of acquisition________________________//
		rp_AcqReset();
		// update sampling rate
		rp_AcqSetSamplingRate(first_band + j%num_bands);
		
		// update buffer_fill_time
		buffer_fill_time = (float) 1000000 * bufsize / (sampling_rates[first_band + j%num_bands]);
		
		printf("sampling rate: %f\n", sampling_rates[first_band + j%num_bands]);
		// debug message
		printf("buffer_fill_time: %f\n", buffer_fill_time);
		
		// make sure buffer is full with new samples	
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
	free(cp);
	free(icp);
	free(cohp);
	rp_AcqStop();	
	return RP_OK;

}	

