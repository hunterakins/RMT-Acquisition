#ifndef __WINDOW_H
#define __WINDOW_H


void GenWindow(int bufsize, float *hp); /*generate hamming window vals */

void Hadamard(int bufsize, float *hp, float *dp); /* componentwise multiply the two arrays and store the result in hp*/ 

#endif
