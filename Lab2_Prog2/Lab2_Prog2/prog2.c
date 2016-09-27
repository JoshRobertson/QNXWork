/*
** fork1.c -- demonstrates usage of fork() and wait()
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>


void sig_handler(int sig);
volatile sig_atomic_t usr1Happened = 0;


int main(void)
{
	pid_t pid;
	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	int rv;
	int numChildren = 0;


	printf("Enter the number of children: ");
	scanf("%d", &numChildren);

	//char n[3];
	//fgets(n, sizeof n, stdin);
	//numbChildren = atoi(n);

	printf("PID = %d : Parent Running...\n", getpid());

	for (int i = 0; i <numChildren; i++){
		switch(pid = fork()){ //sends 0 to the child, and child's pid to the parent
			case -1:
				perror("fork");  /* something went wrong */
				exit(1);		 /* parent exits */

			case 0:
				sigaction(SIGUSR1, &sa, NULL);
				printf("PID = %d : Child Running...\n", getpid());
				while (!usr1Happened){sleep(1);}
		    	printf("PID = %d : Child Received USR1\n", getpid());
		    	printf("PID = %d : Child Exiting.\n", getpid());
		    	printf("Goodbye from pid = %d\n", getpid());
				exit(0);

			default:
				break;
			}
	}

	for (int i = 0; i < numChildren; i++){
		wait(&rv);
	}

	printf("PID = %d : Children Finished, Parent Exiting.\n", getpid());

	return EXIT_SUCCESS;
}

void sig_handler(int sig){
	if (sig == SIGUSR1){
		usr1Happened = 1;
	}
}
