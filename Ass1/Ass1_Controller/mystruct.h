#ifndef MYSTRUCT_H_
#define MYSTRUCT_H_

typedef enum {
	LS = 1,
	RS = 2,
	WS = 3,
	LO = 4,
	RO = 5,
	LC = 6,
	RC = 7,
	GRU = 8,
	GRL = 9,
	GLL = 10,
	GLU = 11
}Event;

typedef struct InputMessage
{
	Event inputEvent;
	int person_id;
	int weight;
	int status;
}InputMessage;


typedef struct DisplayMessage{
	char msg[128];
	int status;
}DisplayMessage;

#endif /* MYSTRUCT_H_ */
