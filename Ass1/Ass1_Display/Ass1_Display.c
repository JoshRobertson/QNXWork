#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include "mystruct.h"

int main(int argc, char* argv[])
{
    int     rcvid;         // indicates who we should reply to
    int     chid;          // the channel ID


    printf("Display PID is %d\n", getpid());

    // create a channel
    chid = ChannelCreate (0);
    if (chid == -1)
    {
    	perror("failed to create the channel.");
    	exit (EXIT_FAILURE);
    }
    // this is typical of a server:  it runs forever
    while (1) {
        StateInput inputFromController;
        StateInput responseToController;
        // get the message, and print it
        rcvid = MsgReceive (chid, &inputFromController, sizeof (inputFromController), NULL);
        //printf ("Got a message, rcvid is %X the length was %d\n", rcvid, sizeof(inputFromController));
        printf ("%s", inputFromController.msg);
        responseToController.status = 0;
        MsgReply(rcvid, EOK, &responseToController, sizeof (responseToController)); // does the display need to reply to the controller? maybe to unblock...
        fflush(stdin);
    }

    // destroy the channel when done
    ChannelDestroy(chid);
   return EXIT_SUCCESS;
}
