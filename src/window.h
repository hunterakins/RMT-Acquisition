#ifndef __WINDOW_H
#define __WINDOW_H


void GenWindow(int bufsize, int16_t *hp); /*generate hamming window vals */

void Hadamard(int bufsize, int16_t *hp, int16_t *dp); /* componentwise multiply the two arrays and store the result in hp*/ 

#endif
