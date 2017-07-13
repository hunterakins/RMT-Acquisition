/* read in the floats from */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv) {
	int bufsize = 16384;
	float *dp = (float *)malloc(bufsize*sizeof(float));
	int i = 0;
	FILE *fd;
	char line[9]; /* array to hold each line of input from file */	
	char *s;
	s = line;

	fd = fopen("data", "r");
	while (fgets(s, 10, fd) != '\0')
		printf("here");
		dp[i] = atof(s);
		i++;
	for (i = 0; i < bufsize; i++) {
		printf("%f\n", dp[i]);
	}
	return 0;
} 


