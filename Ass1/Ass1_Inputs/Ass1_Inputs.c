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

	printf("Enter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, ro=right open, lc = left closed, rc = right closed , "
			"gru = guard right unlock, grl = guard right lock, gll=guard left lock, glu = guard left unlock)\n");

	while(1){
		InputMessage inputToController;
		DisplayMessage responseFromController;
		sleep(2);
		printf("Enter an event: ");
		fgets(inputEvent, sizeof inputEvent, stdin);

		if (strncmp(inputEvent, "ls", (strlen(inputEvent) -1))== 0){
			inputToController.inputEvent = LS;
		}
		else if (strncmp(inputEvent, "rs", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = RS;
		}
		else if (strncmp(inputEvent, "ws", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = WS;
		}
		else if (strncmp(inputEvent, "lo", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = LO;
		}
		else if (strncmp(inputEvent, "ro", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = RO;
		}
		else if (strncmp(inputEvent, "lc", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = LC;
		}
		else if (strncmp(inputEvent, "rc", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = RC;
		}
		else if (strncmp(inputEvent, "gru", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = GRU;
		}
		else if (strncmp(inputEvent, "grl", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = GRL;
		}
		else if (strncmp(inputEvent, "gll", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = GLL;
		}
		else if (strncmp(inputEvent, "glu", (strlen(inputEvent)-1))== 0){
			inputToController.inputEvent = GLU;
		}

		if (inputToController.inputEvent == LS || inputToController.inputEvent == RS){
			printf("Input Person Id: ");
			fgets(inputId, sizeof inputId, stdin);
			inputToController.person_id = atoi(inputId);
		}
		else if (inputToController.inputEvent == WS){
			printf("Input Weight: ");
			fgets(inputWeight, sizeof inputWeight, stdin);
			inputToController.weight = atoi(inputWeight);
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
