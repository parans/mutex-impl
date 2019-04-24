/*Authors Alden and Saketh*/

#ifndef SEM_H
#define SEM_H

#include <stdlib.h>
#include "threads.h"\

#define SEM_MUTEX_INITIALIZER 1 //Always use this to initialize a mutex

struct semaphore
{
	short sem_value; //This value has to be assigned cautiously, it is the programmers responsibility
	Q thread_Q;
};

typedef struct semaphore Semaphore_t;

//Create semaphore and initialize it to @semaphore_value
void CreateSem(Semaphore_t* sem, int semaphore_value)
{
	sem->sem_value = semaphore_value-1;
	sem->thread_Q = malloc(sizeof(struct queue));
	sem->thread_Q->head = NULL;
}

// Deletes tcb from run_Q and moves tcb to semaphore queue, hence momentarily blocking it
void block(Semaphore_t* sem)
{
    TCB_t tcb;
    if(getSchedulerQueue()==NULL)
	exit(-1);
    tcb = softDelTCB(getSchedulerQueue());
    AddQ(sem->thread_Q, tcb);
    return;
}


void P(Semaphore_t* sem)
{
	//while is needed, so that the thread continuosly loops in case it is the only thread in the queue
	while(sem->sem_value<0)
	{
        block(sem);
		yeild();
	}
	sem->sem_value--;
	return;
}

//Removes tcb if any from the semaphore Q and moves it to Run Q
void unblock(Semaphore_t* sem)
{
    TCB_t tcb;

    if(getSchedulerQueue()==NULL)
	exit(-1);

    if(sem->thread_Q->head!=NULL)   //Signal only if some1 is waiting on the sem->Q
    {
        tcb = DelQ(sem->thread_Q);
        tcb->tcb_invalid = 0;
        AddQ(getSchedulerQueue(), tcb);
    }
}

void V(Semaphore_t* sem)
{
	while(sem->sem_value == 0)
	{
        	unblock(sem);
        	yeild(); //This yeild may be to wake the waiting thread or someone on the mainQ
	}
	sem->sem_value++;
	unblock(sem);
	if(sem->sem_value<=0) // This condition prevents unecessary yeilding of the thread which wakes up from the previous yeild
        yeild();
}

#endif
