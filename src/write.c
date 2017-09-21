#include "write.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>



int WriteTimeData(FILE *fd, float *dp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%f\n", *(dp+i));
	}
	return 0;
}

int WriteTwoChannels(FILE *fd, float *dp, float *dp1, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%f\t%f\n", *(dp+i), *(dp1+i));
	}
	return 0;
}
