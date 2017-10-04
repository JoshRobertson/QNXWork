#ifndef GLOBALS_H_
#define GLOBALS_H_

#define MYDEVICE "/dev/local/mydevice"
#define MYCONTROLLER "mycontroller"
#define OPEN 0
#define PULSE 1
#define CLOSE 2
#define NUMPULSES 10
#define NUMINPUTS 3
#define OPEN_S "Open"
#define CLOSED_S "Closed"

char *inputs[NUMINPUTS] = {
		"status open",
		"pulse ",
		"status close"};//enum-esque array of valid inputs

#endif /* GLOBALS_H_ */
