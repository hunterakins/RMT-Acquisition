#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "/opt/redpitaya/include/redpitaya/rp.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
#define CONF_SIZE 100
#define TIME_SIZE 100

char conf_base[] = "/home/redpitaya/RedPitaya/RMT-Acquisition/config/";

int main(int argc, char * argv[]) {	
	if (argc == 1) {
		fprintf(stderr, "no config file supplied\n choose a config file from config dir\n do not supply the full path of the file\njust the name of the file is sufficient\n");
		return 1;
	}
	// _____________________getting conf file info__________________________________//	
	char conf[CONF_SIZE];
	if (strlen(argv[1]) > CONF_SIZE) {
		fprintf(stderr, "config filename limited to %d chars\n", (int) CONF_SIZE);
	}
	if (strcpy(conf, argv[1]) == NULL) {
		fprintf(stderr, "string copy failed");
	}
	if (strcat(conf_base, conf) == NULL ) {
		fprintf(stderr, "string concat/ failed on config file\n");
	}
	config_t cfg;
	config_setting_t *setting;
	
	config_init(&cfg);
	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, conf_base)) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}
	setting = config_lookup(&cfg, "main");
	bool cas = config_setting_get_bool_elem(setting, 0);	
	int write_flag;
	config_lookup_bool(&cfg, "display.write", &write_flag);
	const char * file_folder;
	
	if (write_flag) {
		config_lookup_string(&cfg, "write.file_folder", &file_folder);	
		mkdir(file_folder, 700);
	}
	int two_channel;
	config_lookup_bool(&cfg, "main.two_channel", &two_channel);
	
	int coh;
	config_lookup_bool(&cfg, "main.coherency", &coh);

	if (cas == 1) {	
		if (two_channel) {
			printf("here\n");
			two_channel_cascade(conf_base);
		}
		else {
			cascade(conf_base);
		}
	}
	
	if (coh) {
		coherency(conf_base);
	}
	
	config_destroy(&cfg);
	return(EXIT_SUCCESS);
	/*
	float * dp = (void *) malloc(sizeof(float) * bufsize);
	float * ip = calloc(bufsize, sizeof(float));
	float * domain = (void *) malloc(sizeof(float) * bufsize);
	float * ap = (void *) malloc(sizeof(float) * bufsize);
	float * rf = (void *) malloc(sizeof(float) * bufsize);
	float * imf = (void *) malloc(sizeof(float) * bufsize);
	float * dp1 = (void *) malloc(sizeof(float) * bufsize);
	float * ip1 = calloc(bufsize, sizeof(float));
	float * domain1 = (void *) malloc(sizeof(float) * bufsize);
	float * ap1 = (void *) malloc(sizeof(float) * bufsize);
	float * rf1 = (void *) malloc(sizeof(float) * bufsize);
	float * imf1 = (void *) malloc(sizeof(float) * bufsize);

	
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
	*/
	rp_Release();
	return 0;

}	
