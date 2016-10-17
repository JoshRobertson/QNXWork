#ifndef MYSTRUCT_H_
#define MYSTRUCT_H_

typedef struct request
{
	int a;
	char op;
	int b;
}request;


typedef struct response
{
	double result;
	int status;
	char err[128];

}response;

#endif /* MYSTRUCT_H_ */
