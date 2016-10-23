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
    StateInput inputToController;
    StateInput responseFromController;

	pid_t serverpid = atoi(argv[1]);
    printf("Inputs PID is %d\n", getpid());

	coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0); //connects to the controller
	if (coid == -1) {
		fprintf(stderr, "Couldn't ConnectAttach Input to Controller\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	//////DO USER INPUT HERE IN LOOP HERE//////
	do{
		printf("\n\nEnter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard right lock, gll=guard left lock, glu = guard left unlock)\n");
		printf("Enter a state:");
		fgets(inputToController.state, sizeof(inputToController.state), stdin);

		if (strncmp(inputToController.state, "ls", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = ls;
			printf("Input Person Id: ");
			scanf("%d",&inputToController.person_id);
		}
		else if (strncmp(inputToController.state, "rs", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = rs;
			printf("Input Person Id: ");
			scanf("%d",&inputToController.person_id);
		}
		else if (strncmp(inputToController.state, "ws", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = ws;
			printf("Input Weight: ");
			scanf("%d",&inputToController.weight);
		}
		else if (strncmp(inputToController.state, "lo", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = lo;
		}
		else if (strncmp(inputToController.state, "ro", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = ro;
		}
		else if (strncmp(inputToController.state, "lc", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = lc;
		}
		else if (strncmp(inputToController.state, "rc", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = rc;
		}
		else if (strncmp(inputToController.state, "gru", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = gru;
		}
		else if (strncmp(inputToController.state, "grl", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = grl;
		}
		else if (strncmp(inputToController.state, "gll", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = gll;
		}
		else if (strncmp(inputToController.state, "glu", (strlen(inputToController.state)-1)== 0)){
			inputToController.inputState = glu;
		}

		if (MsgSend(coid, &inputToController, sizeof(inputToController) + 1, &responseFromController,
				sizeof(responseFromController)) == -1) { //sends msg to server and SEND blocks until it gets a reply
			fprintf(stderr, "Error during MsgSend\n");
			perror(NULL);
			exit(EXIT_FAILURE);
		}
		sleep(5);
	}
	while(responseFromController.status != 9); //loop until specific error status returned


	//Disconnect from the channel
	ConnectDetach(coid);

	return EXIT_SUCCESS;
}
