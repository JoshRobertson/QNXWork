#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <process.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <time.h>
#include <sys/dispatch.h>
#include "mystruct.h"

#define DISPLAY_NAME "mydisplay"
#define CONTROLLER_NAME "mycontroller"
#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define MY_MESSG_CODE	_PULSE_CODE_MINAVAIL+1

typedef void *(*StateFunc)(); //sFP is a pointer to a function taking no args and returning void

timer_t timer_id;
struct itimerspec itime;
struct sigevent event;
MyPulseStruct input;
DisplayMessage outputToDisplay;
int     rcvid;         	// indicates who we should reply to
int		rcvidNew;
int 	coid;
name_attach_t *attach;

//State Function Prototypes
void *Start();//, = initialize controller
void *Ready();// – display readyMsg, then wait at least 3 seconds
void *LeftDown();// – display leftMsg
void *RightDown();// – display rightMsg
void *Armed();// – display armedMsg, then wait at least 2 seconds
void *Punch();// – display punchMsg, then wait at least 1 second
void *Exit();// – display exitMsg, then wait at least 5 seconds
void *Stop();// – stop controller

int main(int argc, char* argv[])
{
	if((coid = name_open(DISPLAY_NAME, 0)) < 0) {
		perror("name_open failed");
		return EXIT_FAILURE;
	}

	if((attach = name_attach(NULL, CONTROLLER_NAME, 0)) == NULL) {
		perror("name_attach failure");
		return EXIT_FAILURE;
	}

	StateFunc stateFunc = Start; //initialize to Start state

	while (1) {
		stateFunc = (StateFunc)(*stateFunc)(); //run whatever stateFunc is currently assigned (default IDLE)
		sleep(1);
	}

	name_close(coid);

	return EXIT_SUCCESS;
}

/////State Functions/////
void *Start(){
	printf("Moving to state: Start\n");
	return Ready;
}


void *Ready(){
	printf("Moving to state: Ready\n");
	strcpy(outputToDisplay.msg, "Ready...\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(3);
	while(input.inputmessage.inputEvent != LD && input.inputmessage.inputEvent != S && input.inputmessage.inputEvent !=RD){
		rcvid = MsgReceive (attach->chid, &input.inputmessage, sizeof (input.inputmessage), NULL);
		MsgReply(rcvid, EOK, &input.inputmessage, sizeof(input.inputmessage));
	}
	if (input.inputmessage.inputEvent == S)
		return Exit;
	else if (input.inputmessage.inputEvent == LD)
		return LeftDown;
	else if (input.inputmessage.inputEvent == RD)
		return RightDown;
	else
		return NULL;
}

void *LeftDown(){
	printf("Moving to state: LeftDown\n");
	strcpy(outputToDisplay.msg, "Left button down – press right button to arm press\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while(input.inputmessage.inputEvent != RD && input.inputmessage.inputEvent != LU){
		rcvid = MsgReceive (attach->chid, &input.inputmessage, sizeof (input.inputmessage), NULL);
		if (rcvid == -1){
			perror("MsgReceive in LeftDown");
		}

		MsgReply(rcvid, EOK, &input.inputmessage, sizeof (input.inputmessage));
	}
	if (input.inputmessage.inputEvent == RD)
		return Armed;
	else if (input.inputmessage.inputEvent == LU)
		return Ready;
	else
		return NULL;

}

void *RightDown(){
	printf("Moving to state: RightDown\n");
	strcpy(outputToDisplay.msg, "Right button down – press left button to arm press\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while(input.inputmessage.inputEvent != LD && input.inputmessage.inputEvent != RU){
		rcvid = MsgReceive (attach->chid, &input.inputmessage, sizeof(input.inputmessage), NULL);
		if (rcvid == -1){
			perror("MsgReceive in RightDown");
		}

		MsgReply(rcvid, EOK, &input.inputmessage, sizeof (input.inputmessage));
	}
	if(input.inputmessage.inputEvent == LD)
		return Armed;
	else if(input.inputmessage.inputEvent == RU)
		return Ready;
	else
		return NULL;

}
void *Armed(){
	printf("Moving to state: Armed\n");
	strcpy(outputToDisplay.msg, "DANGER – Press is Armed!\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, attach->chid, _NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = getprio(0);
	event.sigev_code = MY_PULSE_CODE;

	if(timer_create(CLOCK_REALTIME, &event, &timer_id) == -1) {
		perror("timer_create error\n");
		exit(EXIT_FAILURE);
	}

	itime.it_value.tv_sec = 5;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 0;
	itime.it_interval.tv_nsec = 0;
	timer_settime(timer_id, 0, &itime, NULL);

	if(timer_settime(timer_id, 0, &itime, NULL) == -1) {
		perror("timer_settime error\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		// get the message and receive it
		rcvidNew = MsgReceive(attach->chid, &input.inputmessage, sizeof(input.inputmessage), NULL);

		MsgReply(rcvidNew, EOK, &input.inputmessage, sizeof(input.inputmessage));

		if(input.inputmessage.inputEvent == LU)
			return Ready;
		else if(input.inputmessage.inputEvent == RU)
			return Ready;
		else
			return Punch;
	}

	return NULL;
}
void *Punch(){
	printf("Moving to state: Punch\n");
	strcpy(outputToDisplay.msg, "Press Cutting Now\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(1);
	input.inputmessage.inputEvent = NULL;
	return Ready;

}
void *Exit(){
	printf("Moving to state: Exit\n");
	strcpy(outputToDisplay.msg, "Powering down.\n");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &outputToDisplay, sizeof(outputToDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(5);

	return Stop;

}
void *Stop(){
	printf("Moving to state: Stop\n");
	return NULL;
	//exit(EXIT_SUCCESS);
}
