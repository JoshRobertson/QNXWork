#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Globals.h"

char* device_buffer; //
char device_status[10]; //store "open" or "closed"
int coid; //server channel
int msgSend;
int nameClosed;

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	int nb;
	nb = strlen(device_status) + 1;
	printf("nb: %d\n", nb); //debug
	if (ocb->offset == nb)	//test to see if we have already sent the whole message.
		return 0;

	//nb = min(nb, msg->i.nbytes);//Return smaller of the size of our data or the size of the buffer

	_IO_SET_READ_NBYTES(ctp, nb);//Set the number of bytes we will return
	SETIOV(ctp->iov, device_status, nb);	//Copy status into reply buffer.

	//ocb->offset += nb;//update offset into our data used to determine start position for next read.

	if (nb > 0)	//If we are going to send any bytes update the access time for this resource.
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return(_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
    int nb = 0; //number of bytes written
    char* buf;

	if( msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg) )) {
		buf = (char *)(msg+1);
		device_buffer = malloc(strlen(buf)); //if have all the data, copy it into the device buffer
		if (device_buffer == NULL) //check malloc success
			return errno;
		strcpy(device_buffer, buf); //copy buf into device_buffer
		nb = strlen(buf);

	}
	else {
#if 0
		//Get all the data in one big buffer
		char *buf;
		buf = malloc( msg->i.nbytes );
		if (NULL == buf )
		   return ENOMEM;
		nb = resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(*msg));
		nb = write(1, buf, nb ); // fd 1 is stdout
		free(buf);
		if( -1 == nb )
		   return errno;
#else
		//Reuse the same buffer over and over again.
		char buf[1000]; // my hardware buffer
        int count, bytes;
        count = 0;

        while ( count < msg->i.nbytes )
        {
        	bytes = resmgr_msgread( ctp, buf, 1000, count + sizeof(*msg ));
        	if( bytes == -1 )
        	   return errno;
        	bytes = write( 1, buf, bytes ); // fd 1 is standard out
        	if( bytes == -1 )
        	{
        	   if (!count )
        	     return errno;
        	   else
        	      break;
        	}
        	count += bytes;
        }
        nb = count;
#endif
	}
    if (buf != NULL){
    	_IO_SET_WRITE_NBYTES (ctp, nb);

        if (strlen(buf) > 0)
            ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
    }

    return (_RESMGR_NPARTS (0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	return (iofunc_open_default (ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t   *ctp;
	int id;
	int i;

	strncpy(device_status, OPEN_S, sizeof(OPEN_S));//initial status is open
	printf("Device Status (from device): %s\n", device_status); //debug

	dpp = dispatch_create();
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;
	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	id = resmgr_attach(dpp, NULL, MYDEVICE, _FTYPE_ANY, NULL, &connect_funcs, &io_funcs, &ioattr);
	if(id == -1){
		fprintf(stderr, "Error during resmgr_attach\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	ctp = dispatch_context_alloc(dpp);

	while(1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);

		sleep(2);

		for (i = 0; i < NUMINPUTS; i++){
			if(strncmp(inputs[i], device_buffer, strlen(inputs[i])) == 0){ //check if valid input
				if(strncmp(inputs[OPEN], device_buffer, strlen(inputs[OPEN])) == 0){
					strcpy(device_status, OPEN_S);
				}
				else if(strncmp(inputs[CLOSE], device_buffer, strlen(inputs[CLOSE])) == 0){
					strcpy(device_status, CLOSED_S);
				}
				else{ //valid but not open/close so must be a pulse
					//if(strncmp(OPEN_S, device_status, strlen(device_status)) == 0){ //only send pulse if device status is open
						coid = name_open(MYCONTROLLER, 0);// connect to named controller
						if (coid == -1) {
							fprintf(stderr, "Error during name_open\n");
							perror(NULL);
							exit(EXIT_FAILURE);
						}

						msgSend = MsgSendPulse(coid, 63, 1, i);// send pulse max priority
						if (msgSend == -1) {
							fprintf(stderr, "Error sending Pulse\n");
							perror(NULL);
							exit(EXIT_FAILURE);
						}

						nameClosed = name_close(coid);
						if (nameClosed == -1)
							fprintf(stderr, "Error during name_close\n");
							perror(NULL);
							exit(EXIT_FAILURE);
					//}
				}
			}
		}
	}
	free(device_buffer);
	return EXIT_SUCCESS;
}
