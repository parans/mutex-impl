/*Authors Alden and Saketh*/

#ifndef Q_H
#define Q_H

#include "TCB.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct queue{
	TCB_t head;
};

typedef struct queue* Q;

Q InitQ()
{
	Q q= malloc(sizeof(struct queue));
	q->head = NULL;
	return q;
}

void AddQ(Q q, TCB_t newTCB)
{
	if(q==NULL)
	{
		printf("\nQ is uninitialized");
		return;
	}

	TCB_t head = q->head;
	if(head == NULL)
	{
		q->head = newTCB;
		q->head->next = newTCB;
		q->head->previous = newTCB;
	}
	else
	{
		TCB_t previous = head->previous;
		previous->next = newTCB;
		newTCB->previous = previous;
		head->previous = newTCB;
		newTCB->next = head;
	}
}

TCB_t DelQ(Q q)
{
	if(q==NULL)
	{
		printf("\nQ is uninitialized");
		return NULL;
	}

	TCB_t deleted = q->head;
	if(deleted == NULL)
		deleted = NULL;
	else if(q->head == q->head->next)
		q->head = NULL;
	else
	{
		TCB_t previous = q->head->previous;
		TCB_t next = q->head->next;
		previous->next = next;
		next->previous = previous;
		q->head = next;
	}
	return deleted;
}

void RotateQ(Q q)
{
	if(q->head==NULL)
		return;
	q->head = q->head->next;
}

/*This method does not delete a tcb, it markes the tcb for deletion by
setting the tcb_invalid to true. It creates a copy of the head tcb stores the
address of the copy in the current head and returns the copy*/

TCB_t softDelTCB(Q q)
{
    TCB_t tcb_copy = (TCB_t)malloc(sizeof(struct TCB));
    if(q->head!=NULL)
    {
        q->head->tcb_invalid = 1;
        memcpy(tcb_copy, q->head, sizeof(struct TCB));
        q->head->private_data = tcb_copy;
        return tcb_copy;
    }
    else{
        return NULL;
    }

}

#endif
