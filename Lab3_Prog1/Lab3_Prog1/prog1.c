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
void sig_handler(int sig);
void* childWaitFunc(void *arg);
volatile sig_atomic_t usr1Happened;


int main(void)
{
	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	int i;
	int numChildren = 0;
	usr1Happened = 0;
	mySemaphore = sem_open(SNAME, O_CREAT, S_IRWXG | S_IRWXU | S_IRWXO, 0);
	printf("Prog1 PID: %d\n", getpid());
	sigaction(SIGUSR1, &sa, NULL);
	printf("Enter the number of child threads to create: ");
	fflush(0);
	scanf("%d", &numChildren);
	pthread_t* childThreads;

	for (i = 0; i <numChildren; i++){
		pthread_create(&childThreads[i], NULL, &childWaitFunc, NULL);
		//pthread_join(&childThreads[i], NULL);
	}

	while (usr1Happened != 1){
		sleep(1);
	}

	return EXIT_SUCCESS;
}

void sig_handler(int sig){
	if (sig == SIGUSR1){
		usr1Happened = 1;
	}
}

void* childWaitFunc(void *arg){
	printf("This is child thread: %d \n", pthread_self());
	while(1){
		printf("Child thread %d blocked \n", pthread_self());
		sem_wait(mySemaphore);
		printf("Child thread %d unblocked \n", pthread_self());
		sleep(5);
	}
	exit(0);
}
