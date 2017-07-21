#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>



void * myfunc(void *myvar);



int main(int argc, char *argv[]) {


	pthread_t thread1;
	pthread_t thread2;
	char *msg1 = "First thread";
	char *msg2 = "Second thread";
	int ret1;
	int ret2;
	
	ret1 = pthread_create(&thread1, NULL, myfunc, (void *) msg1);
	ret2 = pthread_create(&thread2, NULL, myfunc, (void *) msg2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	printf("main functino after pthread create");

	printf("first thread ret1 = %d\n", ret1);
	printf("first thread ret2 = %d\n", ret2);

	return 0;
}


void * myfunc(void *myvar) {
	char *msg;
	msg = (char *) myvar;

	int i;
	for (i=0; i<10; i++) {
		printf("%s %d\n", msg, i);
		sleep(1);
	}
	return NULL;
}

