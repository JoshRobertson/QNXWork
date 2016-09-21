/*
 ** sigint.c -- grabs SIGINT
 * Source:  http://beej.us/guide/bgipc/output/html/multipage/signals.html
 * Mod. by:  Carolina Ayala
 * compile with -std=gnu99 flag!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int sig);


/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(void) {

	char s[140];
	struct sigaction sa;

	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Enter a string:  ");

	if (fgets(s, sizeof s, stdin) == NULL)
		perror("fgets");
	else
		printf("You entered: %s\n", s);

	return 0;
}

void sigint_handler(int sig) {
	write(0, "\nAhhh! SIGINT!\n", 14);
}
