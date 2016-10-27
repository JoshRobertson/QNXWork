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
	DisplayMessage inputFromController;
	DisplayMessage responseToController;
    printf("Display PID is %d\n", getpid());

    chid = ChannelCreate (0);
    if (chid == -1)
    {
    	perror("failed to create the channel.");
    	exit (EXIT_FAILURE);
    }

    while (1) {
        rcvid = MsgReceive (chid, &inputFromController, sizeof (inputFromController), NULL);
        printf ("%s\n", inputFromController.msg);
        responseToController.status = 0;
        MsgReply(rcvid, EOK, &responseToController, sizeof (responseToController));
        //fflush(stdin);
    }

    // destroy the channel when done
    ChannelDestroy(chid);
   return EXIT_SUCCESS;
}
