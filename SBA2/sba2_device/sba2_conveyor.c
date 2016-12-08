/* * Define THREAD_POOL_PARAM_T such that we can avoid a compiler * warning when we use the dispatch_*() functions below */
#define THREAD_POOL_PARAM_T dispatch_context_t

#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define DISPLAY_NAME "/dev/local/mydisplay"
#define CONTROLLER_NAME "mycontroller"
#define MYDEVICE "/dev/local/mydevice"
#define INPUT "input"

#define CONVEYOR "/dev/local/conveyor"
#define MIXER "/dev/local/mixer"

char* devicename;
char* device_buffer;
char* buffer_pointer;
char device_status[100];
int numMixers;
char* memory_pointer;
int coid;
char* buffer;

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nb;
	nb = strlen(device_status);

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, device_status, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nb;
	int i;
	char* buf;

	buf = (char *) (msg + 1); // skip the first byte
	nb = strlen(buf);
	device_buffer = malloc(strlen(buf));
	if (NULL == device_buffer)
		return ENOMEM;

	strncpy(device_buffer, buf, strlen(buf)); //copy msg into device_buffer
	buffer_pointer = device_buffer; //set pointer to beginning of device_buffer

	for (i = 0; i < strlen(device_buffer); i++){ //check if valid input
		if(strncmp(buffer_pointer, "conveyor on", strlen("conveyor on")) == 0){
			strcpy(device_status, "On");
			buffer_pointer += strlen("conveyor on"); //increment by the size of the string that was matched
		}
		else if(strncmp(buffer_pointer, "conveyor off" , strlen("conveyor off")) == 0){
			strcpy(device_status, "Off");
			buffer_pointer += strlen("conveyor off");
		}
		else if(strncmp(buffer_pointer, "point ", strlen("point ")) == 0){
			buffer_pointer += strlen("point ");
			int x = atoi(buffer_pointer); //x is the mixer number
			if (strncmp(device_status, "On", strlen("On")) == 0){
				printf("Error, device is on\n");
			}
			else if (x > 0 && x < 6){
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


	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS (0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra) {
	return (iofunc_open_default(ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {
	thread_pool_attr_t pool_attr;
	thread_pool_t* tpp;
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	int id;

	//devicename = argv[1];
	dpp = dispatch_create();
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	id = resmgr_attach(dpp, NULL, DISPLAY_NAME, _FTYPE_ANY, NULL, &connect_funcs,	&io_funcs, &ioattr);
	if (id == -1) {
		fprintf(stderr, "Error during resmgr_attach\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	memset(&pool_attr, 0, sizeof pool_attr);
	pool_attr.handle = dpp;
	pool_attr.context_alloc = dispatch_context_alloc;
	pool_attr.block_func = dispatch_block;
	pool_attr.unblock_func = dispatch_unblock;
	pool_attr.handler_func = dispatch_handler;
	pool_attr.context_free = dispatch_context_free;
	pool_attr.lo_water = 2;
	pool_attr.hi_water = 4;
	pool_attr.increment = 1;
	pool_attr.maximum = 50;

	if((tpp = thread_pool_create(&pool_attr, POOL_FLAG_EXIT_SELF)) == NULL) {
		fprintf(stderr, "%s: Unable to initialise thread pool.\n", argv[0]);
		return EXIT_FAILURE;
	}

	thread_pool_start(tpp);

	return EXIT_SUCCESS;
}
