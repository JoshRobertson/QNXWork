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
	char msg_buffer[128]; 	// buffer to read
	union recv_msgs rmsg;	// message to receive
	int size_read; 			// size of buffer read
	name_attach_t *attach;	// structure to hold name space data

	// create local name
	if ((attach = name_attach(NULL, MYCONTROLLER, 0)) == NULL) {
		perror("Cannot attach name in Controller");
		return EXIT_FAILURE;
	}

	// open a file
	fd = open(MYDEVICE, O_RDWR); // CALLS IO_OPEN
	if(fd == -1){
		perror("Error opening device from controller");
		return EXIT_FAILURE;
	}

	// Read initial status of the device
	size_read = read(fd, msg_buffer, sizeof(msg_buffer)); // calls IO_READ
	if (size_read == -1){
		perror("Error on initial read"); //ESRCH : no such process
		printf("%d %s %d\n", fd, msg_buffer, sizeof(msg_buffer));
		return EXIT_FAILURE;
	}
	printf("Initial Device Status: %s\n", msg_buffer);

	while (1) {
		// receive pulse
		if (MsgReceivePulse(attach->chid, &rmsg, sizeof(rmsg), NULL) == 0) {
			// Read device status
			size_read = read(fd, msg_buffer, sizeof(msg_buffer)); // calls IO_READ
			if (size_read == -1){
				perror("Error on read");
				return EXIT_FAILURE;
			}
			// if status is open
			if (strcmp(OPEN_S, msg_buffer) == 0) {//only print pulse if device state is open
				printf("Pulse Value: %d\n", rmsg.pulse.value.sival_int);
			}
			// print status message
			printf("Device Status: %s\n", msg_buffer);
		}
		strcpy(msg_buffer, ""); // reset buffer
	}
	close(fd);
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
