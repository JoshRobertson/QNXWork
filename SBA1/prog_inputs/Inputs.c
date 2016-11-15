#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <unistd.h>
#include "mystruct.h"

int main(int argc, char* argv[])
{
	int coid;
	char inputEvent[5];
	char inputId[20];
	char inputWeight[20];
	pid_t serverpid = atoi(argv[1]);

    printf("Inputs PID is %d\n", getpid());

	coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		fprintf(stderr, "Couldn't ConnectAttach Input to Controller\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	printf("Enter the event type (ld, lu, rd, ru, s)\n");

	while(1){
		InputMessage inputToController;
		DisplayMessage responseFromController;
		sleep(2);
		printf("Enter an event: ");
		fgets(inputEvent, sizeof inputEvent, stdin);

		if (strncmp(inputEvent, "ld", (strlen(inputEvent) -1))== 0){
			inputToController.inputEvent = LD;
		}
		else if (strncmp(inputEvent, "lu", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = LU;
		}
		else if (strncmp(inputEvent, "rd", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = RD;
		}
		else if (strncmp(inputEvent, "ru", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = RU;
		}
		else if (strncmp(inputEvent, "s", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = S;
		}

		if (MsgSend(coid, &inputToController, sizeof(inputToController) + 1, &responseFromController,
				sizeof(responseFromController)) == -1) {
			fprintf(stderr, "Error during MsgSend to Controller\n");
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	};

	//Disconnect from the channel
	ConnectDetach(coid);

	return EXIT_SUCCESS;
}
