#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Globals.h"

char* device_buffer;
char* buffer_pointer;
char device_status[10]; //store "open" or "closed"
int coid; //server channel

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nb;
	nb = strlen(device_status);

	if (ocb->offset == nb)//test to see if we have already sent the whole message.
		return 0;

	nb = min(nb, msg->i.nbytes);


	_IO_SET_READ_NBYTES(ctp, nb);	//Set the number of bytes we will return
	SETIOV(ctp->iov, device_status, nb);	//Copy status into reply buffer.

	ocb->offset += nb;

	if (nb > 0)	//If we are going to send any bytes update the access time for this resource.
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nb; //number of bytes written
	int i;
	char* buf;
	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		buf = (char *) (msg + 1); // skip the first byte
		nb = strlen(buf);
		device_buffer = malloc(strlen(buf));
		if (NULL == device_buffer)
			return ENOMEM;

		strncpy(device_buffer, buf, strlen(buf)); //copy msg into device_buffer
		buffer_pointer = device_buffer; //set pointer to beginning of device_buffer

		for (i = 0; i < strlen(device_buffer); i++){ //check if valid input
			if(strncmp(buffer_pointer, inputs[OPEN], strlen(inputs[OPEN])) == 0){
				strcpy(device_status, OPEN_S);
				buffer_pointer += strlen(inputs[OPEN]); //increment by the size of the string that was matched
			}
			else if(strncmp(buffer_pointer, inputs[CLOSE], strlen(inputs[CLOSE])) == 0){
				strcpy(device_status, CLOSED_S);
				buffer_pointer += strlen(inputs[CLOSE]);
			}
			else if(strncmp(buffer_pointer, inputs[PULSE], strlen(inputs[PULSE])) == 0){
				buffer_pointer += strlen(inputs[PULSE]);
				int x = atoi(buffer_pointer);
				if (x > 0 && x < 11){
					int msgSend = MsgSendPulse(coid, sched_get_priority_max(SCHED_RR), 1, x);// send pulse max priority
					if (msgSend == -1) {
						fprintf(stderr, "Error sending Pulse\n");
						perror(NULL);
						exit(EXIT_FAILURE);
					}
				}
			}
			else{
				buffer_pointer++; //no match, increment by one to check again
			}
		}
	}

	else {
		//Reuse the same buffer over and over again.
		char buf[1000]; // my hardware buffer
		int count, bytes;
		count = 0;

		while (count < msg->i.nbytes) {
			bytes = resmgr_msgread(ctp, buf, 1000, count + sizeof(*msg));
			if (bytes == -1)
				return errno;
			bytes = write(1, buf, bytes); // fd 1 is standard out
			if (bytes == -1) {
				if (!count)
					return errno;
				else
					break;
			}
			count += bytes;
		}
		nb = count;
	}

	_IO_SET_WRITE_NBYTES(ctp, nb); //notify number of bytes that were written

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,	void *extra) {
	return (iofunc_open_default(ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	dpp = dispatch_create();
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;
	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	id = resmgr_attach(dpp, NULL, MYDEVICE, _FTYPE_ANY, NULL, &connect_funcs, &io_funcs, &ioattr);
	if (id == -1) {
		fprintf(stderr, "Error during resmgr_attach\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	coid = name_open(MYCONTROLLER, 0); // connect to named controller
	if (coid == -1) {
		fprintf(stderr, "Error during name_open\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	ctp = dispatch_context_alloc(dpp);

	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}

	int nameClosed = name_close(coid);
	if (nameClosed == -1){
		fprintf(stderr, "Error during name_close\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	free(device_buffer);
	return EXIT_SUCCESS;
}
