#ifndef MYSTRUCT_H_
#define MYSTRUCT_H_

enum State{
	ls,
	rs,
	ws,
	lo,
	ro,
	lc,
	rc,
	gru,
	grl,
	gll,
	glu
}State;

typedef struct StateInput
{
	char state[5];
	enum State inputState;
	int person_id;
	int weight;
	int status;
	char msg[128];
}StateInput;

#endif /* MYSTRUCT_H_ */
