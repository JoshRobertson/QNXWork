#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>

#define SNAME "/mysem"
sem_t *mySemaphore;


int main(void)
{
	int i;
	int numWake = 0;

	printf("Prog2 PID: %d\n", getpid());
	do{
		mySemaphore = sem_open(SNAME, O_CREAT, S_IRWXG | S_IRWXU | S_IRWXO, 0);
		printf("How many threads do you want to wake up (enter 0 to exit)? ");
		fflush(0);
		scanf("%d", &numWake);
		fflush(0);
		sleep(1);
		for (i = 0; i < numWake; i++){
			sem_post(mySemaphore);
		}
	}while(numWake != 0);
	printf("Prog2 Exited Successfully");
	exit(0);
	return EXIT_SUCCESS;
}
