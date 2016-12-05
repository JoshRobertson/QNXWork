#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BEATDEVICE "/dev/local/beatdevice"
#define BEATINPUT "beatinput"
#define B1_S 0
#define B2_S 1
#define B3_S 2
#define D_S  3
#define NUMINPUTS 4

char *inputs[NUMINPUTS] = {
		"b1",
		"b2",
		"b3",
		"d"
};

char userinput[20];
char* filename;
char* devicename;
int isInteractive;
name_attach_t *attach;

int main(int argc, char *argv[]) {
	int speed = 0;
	char* numBells;
	int delayNum = 0;;


	if (argv[1][0] == '-'){//program name (0), -loop (1), filename (2), devicename (3), speed (4)
		isInteractive = 0;
		filename = argv[2];
		devicename = argv[3];
		speed = atoi(argv[4]);
	}
	else{//program name (0), devicename (1), speed (2)
		isInteractive = 1;
		filename = 0;
		devicename = argv[1];
		speed = atoi(argv[2]);
	}


	if(speed < 0 || speed > 5000){
		printf("Invalid speed\n");
		return EXIT_FAILURE;
	}

	attach = name_attach(NULL, BEATINPUT, 0);
	if (attach == NULL) {
		perror("Cannot attach name in beatInput");
		return EXIT_FAILURE;
	}


	while(1){
		if(isInteractive){
			printf("Input a new command:");
			fgets(userinput, sizeof(userinput), stdin );
		}
		else{
			int file = fopen(filename, O_RDONLY);
			fgets(userinput, sizeof(userinput), file);
			fclose(file);
			//loop mode, read from file and start over when eof reached
		}

		if (strncmp(userinput, inputs[B1_S], (strlen(userinput) -1))== 0){
			numBells = "1";
		}
		else if (strncmp(userinput, inputs[B2_S], (strlen(userinput) -1))== 0){
			numBells = "2";
		}
		else if (strncmp(userinput, inputs[B3_S], (strlen(userinput) -1))== 0){
			numBells = "3";
		}
		else if (strncmp(userinput, inputs[D_S], 1)== 0){ //compare 1st char to check for d
			delayNum = atoi(&userinput[1]); //get number for delay
			sleep(delayNum/1000);//"delay for an extra ### number of milliseconds before sending the next beat"
		}
		else{
			continue;
		}

		sleep(speed/1000); // "The speed parameter is the integer number of milliseconds to delay between beats. "

		int fd = open(devicename, O_WRONLY);
		if (fd == -1) {
			perror("Error opening device from controller");
			return EXIT_FAILURE;
		}
		int size_write = write(fd, numBells, strlen(numBells));
		if (size_write == -1) {
			perror("Error on write");
			return EXIT_FAILURE;
		}
		close(fd);
	}
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
