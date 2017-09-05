#include "write.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>



int WriteTimeData(FILE *fd, int16_t *dp, int numvals) {
	int i;
	for (i = 0; i < numvals; i++) {
		fprintf(fd, "%" PRId16 "\n", *(dp+i));
	}
	return 0;
}


