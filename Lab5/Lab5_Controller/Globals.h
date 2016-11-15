#ifndef GLOBALS_H_
#define GLOBALS_H_

#define MYDEVICE "/dev/local/mydevice"
#define MYCONTROLLER "dev/local/mycontroller"
#define OPEN 0
#define CLOSE 11
#define NUMPULSES 10
#define NUMINPUTS 12
#define OPEN_S "Open"
#define CLOSED_S "Closed"

char *inputs[NUMINPUTS] = {
		"status open",
		"pulse 1",
		"pulse 2",
		"pulse 3",
		"pulse 4",
		"pulse 5",
		"pulse 6",
		"pulse 7",
		"pulse 8",
		"pulse 9",
		"pulse 10",
		"status close"};//enum-esque array of valid inputs

// message to be received
union recv_msgs {
	struct _pulse pulse;
	uint16_t type;
};

#endif /* GLOBALS_H_ */
