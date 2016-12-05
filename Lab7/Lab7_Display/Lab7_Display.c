#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <process.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/dispatch.h>
#include "mystruct.h"

#define DISPLAY_NAME "mydisplay"
#define CONTROLLER_NAME "mycontroller"

int msgrcvid;
DisplayMessage displayMessage = {0};

int main(int argc, char* argv[])
{
	name_attach_t *attach;

	if((attach = name_attach(NULL, DISPLAY_NAME, 0)) == NULL) {
		perror("name_attach failure");
		return EXIT_FAILURE;
	}

	while(1) {
		// get the message and receive it
		msgrcvid = MsgReceive(attach->chid, &displayMessage, sizeof(displayMessage), NULL);
		if (msgrcvid == -1){
			perror("MsgReceive in Display");
		}
		printf("%s", displayMessage.msg);

		// prepare reply
		MsgReply(msgrcvid, EOK, &displayMessage, sizeof(displayMessage));
	}

	return EXIT_SUCCESS;
}
