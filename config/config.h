#include <stdio.h>
#include <stdbool.h>
// idea is to use header as config file

// cascaded acquisition, e.g. 4 different sampling rates
extern bool CASCADE;
// perform processing 
extern bool PROCESS;
// 

// what to do with the output?
typedef struct {
	// write to file
	bool WRITE;
	bool DISPLAY;
	char *FILEBASE;
} OUTPUT_t;


typedef struct {
	bool FFT;
	bool CROSSPOWER;
	bool COHERENCY;
} PROC_t;	


