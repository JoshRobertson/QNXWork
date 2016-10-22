#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include "mystruct.h"
#include <limits.h>
#include <float.h>

int main (int argc, char* argv[])
{
    int     rcvid;         // indicates who we should reply to
    int     chid;          // the channel ID
    request myRequest;
    response myResponse;

    // create a channel
    chid = ChannelCreate (0);
    if (chid == -1)
    {
    	perror("failed to create the channel.");
    	exit (EXIT_FAILURE);
    }
    printf("Server PID is %d\n", getpid());
    // this is typical of a server:  it runs forever
    while (1) {
        // get the message, and print it
        rcvid = MsgReceive (chid, &myRequest, sizeof (myRequest), NULL);
        //printf ("Got a message, rcvid is %X the length was %d\n", rcvid, sizeof(myRequest));
        //printf ("Message was \"%d %c %d\".\n", myRequest.a, myRequest.op, myRequest.b);

        char operator = myRequest.op;
        double a = myRequest.a;
        double b = myRequest.b;

        if(operator == '/'){
        	if (myRequest.b == 0){
        		strcpy(myResponse.err, "You destroyed the universe");
        		myResponse.status = -1;
        	}
        	else {
        		myResponse.result = a / b;
        		myResponse.status = 0;
        	}
        }
        else if (operator == 'x' || operator == '*'){
        	myResponse.result = a * b;
        	myResponse.status = 0;
        }
        else if (operator == '-'){
        	myResponse.result = a - b;
        	myResponse.status = 0;
        }
        else if (operator == '+'){
        	myResponse.result = a + b;
        	myResponse.status = 0;
        }

        if (myResponse.result > FLT_MAX){
        	strcpy(myResponse.err, "Come on. That's just too big.");
        	myResponse.status = -1;
        }

        MsgReply(rcvid, EOK, &myResponse, sizeof (myResponse));
    }

    // destroy the channel when done
    ChannelDestroy(chid);
   return EXIT_SUCCESS;
}
