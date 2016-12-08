#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <process.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "mystruct.h"

#define DISPLAY_NAME "/dev/local/mydisplay"
#define CONTROLLER_NAME "mycontroller"
#define MYDEVICE "/dev/local/mydevice"
#define INPUT "input"
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define MY_MESSG_CODE	_PULSE_CODE_MINAVAIL+1


MyPulseStruct inputToController;
int coid;
int fd;
char inputEvent[5];

int main(int argc, char* argv[])
{
	coid = name_open(CONTROLLER_NAME, 0);
	if(coid < 0) {
		perror("name_open failed");
		return EXIT_FAILURE;
	}
	printf("Enter the event type (ld, lu, rd, ru, s)\n");

	while(1){
		fgets(inputEvent, sizeof inputEvent, stdin);

		if (strncmp(inputEvent, "ld", (strlen(inputEvent) -1))== 0){
			inputToController.inputmessage.inputEvent = LD;
		}
		else if (strncmp(inputEvent, "lu", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = LU;
		}
		else if (strncmp(inputEvent, "rd", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = RD;
		}
		else if (strncmp(inputEvent, "ru", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = RU;
		}
		else if (strncmp(inputEvent, "s", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = S;
		}
		else{
			continue;
		}

		inputToController.pulse.code = MY_MESSG_CODE;

		if (MsgSend(coid, &inputToController.inputmessage, sizeof(inputToController.inputmessage) + 1, &inputToController.inputmessage,
				sizeof(inputToController.inputmessage)) == -1) {
			fprintf(stderr, "Error during MsgSend to Controller\n");
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}

	//Disconnect from the channel
	name_close(coid);

	return EXIT_SUCCESS;
}
