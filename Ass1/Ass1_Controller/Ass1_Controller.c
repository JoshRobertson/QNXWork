#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "mystruct.h"

int main(int argc, char* argv[])
{
	int coid;
	int     rcvid;         // indicates who we should reply to
    int     chid;          // the channel ID

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

    // this is typical of a server:  it runs forever
    while (1) {
        StateInput input;
        StateInput outputToDisplay;
        StateInput responseToInputs;
        StateInput responseFromDisplay;

        rcvid = MsgReceive (chid, &input, sizeof (input), NULL);
        responseToInputs.status = 0;

        switch(input.inputState){
        case ls:
        	sprintf(outputToDisplay.msg,"Left Swipe. Id # %d\n", input.person_id);
        	break;
        case rs:
        	sprintf(outputToDisplay.msg,"Right Swipe. Id # %d\n", input.person_id);
        	break;
        case ws:
        	sprintf(outputToDisplay.msg,"Weight Scale. Weight is: %d lbs\n", input.weight);
        	break;
        case lo:
        	sprintf(outputToDisplay.msg,"Left Door Open\n");
        	break;
        case ro:
        	sprintf(outputToDisplay.msg,"Right Door Open\n");
        	break;
        case lc:
        	sprintf(outputToDisplay.msg,"Left Door Closed\n");
        	break;
        case rc:
        	sprintf(outputToDisplay.msg,"Right Door Closed\n");
        	break;
        case gru:
        	sprintf(outputToDisplay.msg,"Guard Unlocked Right Door\n");
        	break;
        case grl:
        	sprintf(outputToDisplay.msg,"Guard Locked Right Door\n");
        	break;
        case gll:
        	sprintf(outputToDisplay.msg,"Guard Locked Left Door\n");
        	break;
        case glu:
        	sprintf(outputToDisplay.msg,"Guard Unlocked Left Door");
        	break;
        default:
        	responseToInputs.status = 9; //invalid state
        	sprintf(outputToDisplay.msg, "Error: Invalid State");
        	break;
        }
        MsgReply(rcvid, EOK, &responseToInputs, sizeof (responseToInputs)); //reply to inputs to say ok?
    	// send the message to display
    	if (MsgSend(coid, &outputToDisplay, sizeof(outputToDisplay) + 1, &responseFromDisplay, sizeof(responseFromDisplay)) == -1) { //sends msg to server and SEND blocks until it gets a reply
    		fprintf(stderr, "Error during MsgSend\n");
    		perror(NULL);
    		exit(EXIT_FAILURE);
    	}
    }
	//Disconnect from the output display channel
	ConnectDetach(coid);
    // destroy the input channel when done
    ChannelDestroy(chid);
   return EXIT_SUCCESS;
}
