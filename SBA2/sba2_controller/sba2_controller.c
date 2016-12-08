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

#define CONVEYOR "/dev/local/conveyor"
#define MIXER "/dev/local/mixer"
#define DISPLAY_NAME "/dev/local/mydisplay"
#define CONTROLLER_NAME "mycontroller"
#define MYDEVICE "/dev/local/mydevice"
#define INPUT "input"
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
int fd, wd, mixerNum;
int conveyorIsOff = 0; //0 == Off, 1 == On
int mixerIsOff = 0;
int pumpIsOff = 0;
name_attach_t *attach;

//State Function Prototypes
void *Start();//, = initialize controller
void *Reset();
void *PointAtX();
void *ConveyorOn();
void *MixOnX();
void *PumpOnX();
void *Stop();// – stop controller

int main(int argc, char* argv[])
{
	attach = name_attach(NULL, CONTROLLER_NAME, 0);

	if (attach == NULL){
		perror("name_attach failure");
		return EXIT_FAILURE;
	}

	StateFunc stateFunc = Start; //initialize to Start state

	while (1) {
		stateFunc = (StateFunc)(*stateFunc)(); //run whatever stateFunc is currently assigned (default IDLE)
		sleep(1);
	}

	return EXIT_SUCCESS;
}

/////State Functions/////
void *Start(){
	printf("Moving to state: Start\n");

	while(input.inputmessage.inputEvent != S && input.inputmessage.inputEvent != S){
		rcvid = MsgReceive (attach->chid, &input.inputmessage, sizeof (input.inputmessage), NULL);
		MsgReply(rcvid, EOK, &input.inputmessage, sizeof(input.inputmessage));
	}
	mixerNum = input.inputmessage.inputMixer;

	return Reset;
}

void *Reset(){
	printf("Mixer: %d, Moving to state: Reset\n", mixerNum);
	strcpy(outputToDisplay.msg, "conveyor off 1");

	fd = open(CONVEYOR, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;
	}
	close(fd);


	strcpy(outputToDisplay.msg, "mix off 1");

	fd = open(MIXER, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;
	}
	close(fd);

	strcpy(outputToDisplay.msg, "pump off 1");

	fd = open(MIXER, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;
	}
	close(fd);
	conveyorIsOff = 0;
	mixerIsOff = 0;
	pumpIsOff = 0;
	return PointAtX;
}

void *PointAtX(){
	printf("Mixer: %d, Moving to state: PointAtX\n", mixerNum);
	strcpy(outputToDisplay.msg, "point 1");

	fd = open(CONVEYOR, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;
	}
	close(fd);
	return ConveyorOn;
}

void *ConveyorOn(){
	printf("Mixer: %d, Moving to state: ConveyorOn\n", mixerNum);

	strcpy(outputToDisplay.msg, "conveyor on");

	fd = open(CONVEYOR, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;
	}
	close(fd);
	conveyorIsOff = 1;
	return MixOnX;
}
void *MixOnX(){
	strcpy(outputToDisplay.msg, "mixer on");

	fd = open(MIXER, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;

	}
	close(fd);
	mixerIsOff = 1;
	printf("Mixer: %d, Moving to state: MixOnX\n", mixerNum);
	return PumpOnX;
}
void *PumpOnX(){
	printf("Mixer: %d, Moving to state: PumpOnX\n", mixerNum);
	strcpy(outputToDisplay.msg, "pump on 1");
	fd = open(MIXER, O_WRONLY);
	if(fd == -1){
		perror("open error in controller\n");
		return NULL;
	}

	wd = write(fd, outputToDisplay.msg, strlen(outputToDisplay.msg));
	if(wd == -1){
		perror("write error in controller\n");
		return NULL;

	}
	pumpIsOff = 1;
	close(fd);
	return Stop;
}

void *Stop(){
	printf("Mixer: %d, Moving to state: Stop\n", mixerNum);

	return NULL;
}

