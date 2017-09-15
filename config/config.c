#include <stdio.h>
#include <stdbool.h>
#include "config.h"
// idea is to use header as config file



int main(){
		CASCADE=true;
		PROCESS=false;

		OUTPUT_t OUTPUT;
		OUTPUT.WRITE=true;
		OUTPUT.DISPLAY=false;
		OUTPUT.FILEBASE="/media/usb";

		printf("%d\n", OUTPUT.DISPLAY);
	return 0;
}

	


