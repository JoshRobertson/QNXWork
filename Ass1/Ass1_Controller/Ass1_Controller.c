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

sig_atomic_t persistWeightEntry = 0;
sig_atomic_t persistIdEntry = 0;
sig_atomic_t persistWeightExit = 0;
sig_atomic_t persistIdExit = 0;
int     rcvid;         	// indicates who we should reply to
int     chid;          	// the channel ID
int 	coid;

//State Function Prototypes
void *IDLE(); 			//can L_ENTRY or R_EXIT
void *L_ENTRY();
void *L_UNLOCK_ENTRY();
void *L_OPEN_ENTRY();
void *L_WOE();
void *L_CLS_ENTRY();
void *L_LOCK_ENTRY();
void *R_UNLOCK_ENTRY();
void *R_OPEN_ENTRY();
void *R_CLS_ENTRY();
void *R_LOCK_ENTRY();
void *R_EXIT();
void *R_UNLOCK_EXIT();
void *R_OPEN_EXIT();
void *R_WOE();
void *R_CLS_EXIT();
void *R_LOCK_EXIT();		//can L_UNLOCK_EXIT or ERROR_ALERT
void *L_UNLOCK_EXIT();
void *L_OPEN_EXIT();
void *L_CLS_EXIT();
void *L_LOCK_EXIT();
void *ERROR_ALERT();

int main(int argc, char* argv[])
{
	pid_t serverpid = atoi(argv[1]);
    printf("Controller PID is %d\n", getpid());

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

	StateFunc stateFunc = IDLE; //initialize to IDLE state

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


//State Functions
void *IDLE(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "System is idle. Both doors locked, and chamber is empty");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while(input.inputEvent != LS && input.inputEvent != RS){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	if (input.inputEvent == LS)
		return L_ENTRY;
	else
		return R_EXIT;
}

void *L_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Left Door Request for Entry. ID#: %d",input.person_id);
	persistIdEntry = input.person_id;
	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GLU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_UNLOCK_ENTRY;
}

void *L_UNLOCK_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Left Door Unlocked before Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != LO){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_OPEN_ENTRY;
}
void *L_OPEN_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Left Door Open before Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != WS){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_WOE;
}
void *L_WOE(){
	persistWeightEntry = input.weight; //keep weight for comparing on exit
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Weight on Entry is: %d", input.weight);

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != LC){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_CLS_ENTRY;
}
void *L_CLS_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Left Door Closed after Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GLL){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_LOCK_ENTRY;
}
void *L_LOCK_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Left Door Locked after Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GRU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_UNLOCK_ENTRY;
}
void *R_UNLOCK_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Right Door Unlocked after Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != RO){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_OPEN_ENTRY;
}
void *R_OPEN_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Right Door Open after Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != RC){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_CLS_ENTRY;
}
void *R_CLS_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Right Door Closed after Entry");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GRL){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_LOCK_ENTRY;
}
void *R_LOCK_ENTRY(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Right Door Locked after Entry.");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	return IDLE; //auto transition to IDLE
}

///START RIGHT FLOW////
void *R_EXIT(){
	persistIdExit = input.person_id;
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Right Door Request for Exit. ID#: %d", input.person_id);

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GRU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_UNLOCK_EXIT;
}
void *R_UNLOCK_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Right Door Unlocked before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != RO){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_OPEN_EXIT;
}
void *R_OPEN_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Right Door Open before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != WS){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_WOE;
}
void *R_WOE(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Weight on Exit is: %d", input.weight);
	persistWeightExit = input.weight;

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != RC){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}
	return R_CLS_EXIT;
}
void *R_CLS_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Right Door Closed before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GRL){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return R_LOCK_EXIT;
}
void *R_LOCK_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Right Door Locked before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GLU){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}


	if(persistWeightEntry != persistWeightExit &&  persistIdEntry == persistIdExit){
		return ERROR_ALERT; //if weights don't match for the same ID #, trigger alert state and transition to locked (IDLE) state
	}
	else{
		return L_UNLOCK_EXIT;
	}
}
void *L_UNLOCK_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Left Door Unlocked before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != LO){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_OPEN_EXIT;
}
void *L_OPEN_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg),"Left Door Open before Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != LC){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_CLS_EXIT;
}
void *L_CLS_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Left Door Closed after Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (input.inputEvent != GLL){
		rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
		if (rcvid == -1){
			perror("MsgReceive");
		}

	    MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs));
	}

	return L_LOCK_EXIT;
}
void *L_LOCK_EXIT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "Left Door Locked after Exit");

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	return IDLE; //Auto transition to IDLE
}
void *ERROR_ALERT(){
	snprintf(outputToDisplay.msg, sizeof(outputToDisplay.msg), "ALERT! POTENTIAL PROBLEM HAS OCCURRED. EXIT WEIGHT FOR ID#%d DOES NOT MATCH ENTRY WEIGHT.", persistIdExit);

	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) {
		fprintf(stderr, "Error during MsgSend to Display\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	return IDLE;
}




