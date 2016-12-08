#ifndef MYSTRUCT_H_
#define MYSTRUCT_H_

typedef enum {
	S = 1,
	P = 2,
	R = 3,
	C = 4,
}Event;

typedef struct DisplayMessage{
	char msg[128];
}DisplayMessage;

typedef struct InputMessage
{
	Event inputEvent;
	int inputMixer;
}InputMessage;

typedef struct MyPulseStruct {
	struct _pulse pulse;
	InputMessage inputmessage;
} MyPulseStruct;


#endif
