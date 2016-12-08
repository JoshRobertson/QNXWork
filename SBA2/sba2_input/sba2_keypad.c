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

#define CONVEYOR "/dev/local/conveyor"
#define MIXER "/dev/local/mixer"
#define DISPLAY_NAME "/dev/local/mydisplay"
#define CONTROLLER_NAME "mycontroller"
#define MYDEVICE "/dev/local/mydevice"
#define INPUT "input"
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define MY_MESSG_CODE	_PULSE_CODE_MINAVAIL+1


MyPulseStruct inputToController;
int coid;
int fd;
int mixerNum;
char inputEvent[5];

int main(int argc, char* argv[])
{
	coid = name_open(CONTROLLER_NAME, 0);
	if(coid < 0) {
		perror("name_open failed");
		return EXIT_FAILURE;
	}

	mixerNum = argv[1];

	printf("Enter the input for mixer %d: S (Start), P (Pause), R (Resume), or C (Cancel).\n", mixerNum);

	while(1){
		fgets(inputEvent, sizeof inputEvent, stdin);

		if (strncmp(inputEvent, "S", (strlen(inputEvent) -1))== 0){
			inputToController.inputmessage.inputEvent = S;
		}
		else if (strncmp(inputEvent, "P", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = P;
		}
		else if (strncmp(inputEvent, "R", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = R;
		}
		else if (strncmp(inputEvent, "C", (strlen(inputEvent)-1))== 0){
			inputToController.inputmessage.inputEvent = C;
		}
		else{
			continue;
		}
		inputToController.inputmessage.inputMixer = mixerNum;
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
