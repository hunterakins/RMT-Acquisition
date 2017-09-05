#include "lin_fit.h"
#include "window.h"
#include "fft.h"

#define BUFSIZE 100

int main() {
	int i = 0;
	size_t bufsize = BUFSIZE;
	int16_t *dp = (int16_t *) (malloc(bufsize * sizeof(int16_t)));
	int16_t *domain = (int16_t *) (malloc(bufsize * sizeof(int16_t)));
	int16_t *ip = calloc(bufsize, sizeof(int16_t));

	

	for (i = 0; i < bufsize; i++) {
		*(dp+i) = i*i;
		printf("%d\n", *(dp + i));
	}
		
	int16_t c0 =0;
	int16_t c1 =0;
	LinearFilter(domain, dp, bufsize, c0, c1);
	for (i = 0; i < bufsize; i++) {
		printf("%d\n", *(dp + i));
	}
	 
	return 0;
	
	GenWindow(bufsize, domain);
	
	Hadamard(bufsize, dp, domain);	

	for (i = 0; i < 100; i++) {
		printf("%d\n", *(dp +i));
	}
	
	FFT(1, 14, dp, ip);	

	
}
	
