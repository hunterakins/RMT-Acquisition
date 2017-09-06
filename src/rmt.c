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

#define RP_BUF_SIZE 16384

int main() {	
	size_t bufsize = RP_BUF_SIZE;
	int16_t * dp = (void *) malloc(sizeof(int16_t) * bufsize);
	
	struct Data Channel1;		



	int i;

	rp_Init();
	for (i=0;i<bufsize;i++) {
		*(dp + i) = i;
	}
	
	Channel1.dp = dp;
	printf("%" PRId16 "\n", *(Channel1.dp +10));	
	free(dp);

	rp_Release();
	return 0;

	
}


