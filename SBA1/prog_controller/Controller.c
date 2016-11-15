#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "mystruct.h"

typedef void *(*StateFunc)(); //sFP is a pointer to a function taking no args and returning void

InputMessage input;
DisplayMessage outputToDisplay;
DisplayMessage responseToInputs;
DisplayMessage responseFromDisplay;

sig_atomic_t leftPushed = 0;
sig_atomic_t rightPushed = 0;

int     rcvid;         	// indicates who we should reply to
int     chid;          	// the channel ID
int 	coid;

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
	pid_t serverpid = atoi(argv[1]);
    printf("Controller startup: PID = %d\n", getpid());

    // create a channel for the input
    chid = ChannelCreate (0);
    if (chid == -1)
    {
    	perror("failed to create the channel.");
    	exit (EXIT_FAILURE);
    }

    //Connect to output display channel
	coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		fprintf(stderr, "Couldn't ConnectAttach Controller to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	StateFunc stateFunc = Start; //initialize to IDLE state

    while (1) {
       stateFunc = (StateFunc)(*stateFunc)(); //run whatever stateFunc is currently assigned (default IDLE)
       //sleep(1);
    }
	//Disconnect from the output display channel
	ConnectDetach(coid);
    // destroy the input channel when done
    ChannelDestroy(chid);
   return EXIT_SUCCESS;
}

/////State Functions/////
void *Start(){
	//snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Moving to state: Start "); //already in the state, but ehhh good enough
	printf("Moving to state: Start\n");
	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	return Ready;
}


void *Ready(){
	printf("Moving to state: Ready\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Ready...");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(3);
	while(input.inputEvent != LD && input.inputEvent != S && input.inputEvent !=RD){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}
		if (input.inputEvent == S)
			return Exit;
		else if (input.inputEvent == LD)
			return LeftDown;
		else
			return RightDown;

}

void *LeftDown(){
	printf("Moving to state: LefDown\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Left button down – press right button to arm press");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while(input.inputEvent != RD && input.inputEvent != LU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}
	if (input.inputEvent == RD)
		return Armed;
	else
		return Ready;

}

void *RightDown(){
	printf("Moving to state: RightDown\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Right button down – press left button to arm press");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while(input.inputEvent != LD && input.inputEvent != RU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}
	if(input.inputEvent == LD)
		return Armed;
	else
		return Ready;

}
void *Armed(){
	printf("Moving to state: Armed\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "DANGER – Press is Armed!");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(2);

	return Punch;
}
void *Punch(){
	printf("Moving to state: Punch\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Press Cutting Now");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(1);

	return Ready;

}
void *Exit(){
	printf("Moving to state: Exit\n");
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Powering down.");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sleep(5);

	return Stop;

}
void *Stop(){
	printf("Moving to state: Stop\n");
	exit(EXIT_SUCCESS);
}
