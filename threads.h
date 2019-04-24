/* Authors Alden and Saketh */

#ifndef THREADS_H
#define THREADS_H

#include <string.h>
#include "q.h"

#define STACK_SIZE 8192

Q tcb_queue = NULL;

// Returns a reference to the global run Q

Q getSchedulerQueue()
{
	return tcb_queue;
}

short getCurrentThreadId()
{
	short threadId = -1;
	if(tcb_queue!=NULL)
		threadId=tcb_queue->head->thread_id;
	return threadId;
}

void init_TCB (TCB_t tcb, void *function, void *stackP, int stack_size)
{
        static short thread_id;
        memset(tcb, '\0', sizeof(struct TCB));
    	getcontext(&tcb->context);
    	tcb->thread_id = ++thread_id;
    	tcb->tcb_invalid = 0;
    	tcb->private_data = NULL;
    	tcb->context.uc_stack.ss_sp = stackP;
    	tcb->context.uc_stack.ss_size = (size_t) stack_size;
    	makecontext(&tcb->context, function, 0);
}

short startThread(void (*function)(void*))
{
	void* sp = malloc(STACK_SIZE);
	TCB_t tcb = (TCB_t)malloc(sizeof(struct TCB));
	if(sp==NULL||tcb==NULL)
		return -1;

	init_TCB(tcb, function, sp, STACK_SIZE);

	if(tcb_queue==NULL)
		tcb_queue = InitQ();
	AddQ(tcb_queue, tcb);
	return 0;
}

void run()
{
    ucontext_t parent;
    getcontext(&parent);
    TCB_t tcb = tcb_queue->head;
    if(tcb!=NULL)
    	swapcontext(&parent, &tcb->context);
}

void yeild()
{
	ucontext_t current_context;
	if(tcb_queue->head!=NULL)
	{
        	getcontext(&current_context);
        	if(tcb_queue->head->tcb_invalid == 1)
        	{
			/*If marked as invalid, then copy the current context to address pointed in private data
			and then delete the tcb from runQ, else */
			((TCB_t)(tcb_queue->head->private_data))->context = current_context;
            		DelQ(tcb_queue);
        	}
        	else
        	{
            		tcb_queue->head->context = current_context;
            		RotateQ(tcb_queue);
        	}
        	swapcontext(&current_context, &(tcb_queue->head)->context);
	}
	else
	{
        	printf("\nQ empty, nothing to yeild to");
        	exit(-1);
	}
}

#endif
