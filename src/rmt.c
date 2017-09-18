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

int main(int argc, char * argv[]) {	

	printf("%s\n", OUTPUT.FILEBASE);

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

	int acquire_time = 20;
	cascade(&acquire_time);

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
