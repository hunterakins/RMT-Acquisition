#include "write.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>



int WriteTimeData(FILE *fd, double *dp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%f\n", *(dp+i));
	}
	return 0;
}


