/*Authors Alden and Saketh*/

#ifndef TCB_H
#define TCB_H

#include <ucontext.h>

struct TCB
{
	struct TCB* previous;
	struct TCB* next;
	short thread_id;
	short tcb_invalid;	//This feild used to mark a TCB for deletion
	ucontext_t context;
	void* private_data;	//void pointer used to store private data
};

typedef struct TCB* TCB_t;

#endif
