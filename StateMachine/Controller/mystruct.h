#ifndef MYSTRUCT_H_
#define MYSTRUCT_H_

typedef enum {
	LD = 1,
	LU = 2,
	RD = 3,
	RU = 4,
	S = 5,
}Event;

typedef enum {
	StartState,//, - initialize controller
	ReadyState,// - display readyMsg, then wait at least 3 seconds
	LeftDownState,// - display leftMsg
	RightDownState,// - display rightMsg
	ArmedState,// - display armedMsg, then wait at least 2 seconds
	PunchState,// - display punchMsg, then wait at least 1 second
	ExitState,// - display exitMsg, then wait at least 5 seconds
	StopState,// - stop controller
}State;

typedef struct DisplayMessage{
	char msg[128];
}DisplayMessage;

typedef struct InputMessage
{
	Event inputEvent;
}InputMessage;

typedef struct MyPulseStruct {
	struct _pulse pulse;
	InputMessage inputmessage;
} MyPulseStruct;


#endif
