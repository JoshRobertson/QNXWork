#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "Globals.h"

int main(void) {
	int fd; 				// file descriptor
	int mrp;				//message receive pulse
	int size_read; 			// size of buffer read
	char msg_buffer[128] = ""; 	// buffer to read
	union recv_msgs rmsg;	// message to receive
	name_attach_t *attach;	// structure to hold name space data

	// create local name
	attach = name_attach(NULL, MYCONTROLLER, 0);
	if (attach == NULL) {
		perror("Cannot attach name in Controller");
		return EXIT_FAILURE;
	}

	while (1) {
		// receive pulse
		mrp = MsgReceivePulse(attach->chid, &rmsg, sizeof(rmsg), NULL);

		if (mrp == -1){
			perror("Error receiving pulse in controller");
			return EXIT_FAILURE;
		}

		fd = open(MYDEVICE, O_RDONLY); // CALLS IO_OPEN
		if (fd == -1) {
			perror("Error opening device from controller");
			return EXIT_FAILURE;
		}
		size_read = read(fd, msg_buffer, sizeof(msg_buffer)); // calls IO_READ
		if (size_read == -1) {
			perror("Error on read");
			return EXIT_FAILURE;
		}

		printf("Pulse Value: %d\n", rmsg.pulse.value.sival_int);
		printf("Device Status: %s\n", msg_buffer);

		close(fd);

		strcpy(msg_buffer, ""); // reset buffer
	}
	//	close(fd);
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
