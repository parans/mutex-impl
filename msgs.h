//Authors Alden and Saketh
#ifndef MSGS_H
#define MSGS_H

#include "sem.h"

#define MESSAGE_LENGTH 10
#define MESSAGE_Q_LENGTH 10
#define SERVER_PORT 80
#define MAX_PORTS 4

#define PRINT 0
#define ADD 1
#define DELETE 2
#define MODIFY 3

#define LOCK 100
#define UNLOCK 101
#define LOCKED 100
#define UNLOCKED 101
/**************Data structures***********************/
//Message structure
struct message
{
	char msg[10];
	int operation;
    int total_size;
	int error_code;
	int row;
	int reply_port;
};

typedef struct message* Message_t;

//This is a message queue stucture
struct message_queue
{
	short in;
	short out;
	short msg_count; //to keep track of the no of messages in a queue, decisons are done based on this
	Message_t* mq;
};

typedef struct message_queue* MessageQ;

//port structure
struct port
{
	MessageQ q;
	int port_number;
	int initialized;
	//int sem_id;
	Semaphore_t* full;
	Semaphore_t* empty;
	Semaphore_t* mutex;
};

typedef struct port* PORT;



PORT ports[MAX_PORTS];

/*****************End of data structures********************************/

PORT getPort(int port_number)
{
	if(ports[port_number] == NULL)
		return NULL;
	return ports[port_number];
}

void printMsg(Message_t msg)
{
    printf("%s\t, code:%d", msg->msg, msg->error_code);
	printf("\n");
}

//Initialize the port data structure
int openPort(int port_number)
{
	PORT p;
	if(ports[port_number]==NULL)
	{
        ports[port_number] = (PORT)malloc(sizeof(struct port));
        if(ports[port_number]->q==NULL)
        {
            ports[port_number]->q = (MessageQ)malloc(sizeof(struct message_queue));
            ports[port_number]->q->in = 0;
            ports[port_number]->q->out = 0;
            ports[port_number]->q->msg_count = 0;
            ports[port_number]->q->mq = (Message_t*)malloc(MESSAGE_Q_LENGTH*sizeof(struct message));

            ports[port_number]->port_number = port_number;

            //ports[port_number]->sem_id = sem;
            p = ports[port_number];
            if(p->empty == NULL)
            {
                p->empty = (Semaphore_t*)malloc(sizeof(Semaphore_t));
                CreateSem(p->empty, MESSAGE_Q_LENGTH);
            }
            if(p->full ==NULL)
            {
                 p->full = (Semaphore_t*)malloc(sizeof(Semaphore_t));
                 CreateSem(p->full, 0);
            }
            if(p->mutex ==NULL)
            {
                p->mutex = (Semaphore_t*)malloc(sizeof(Semaphore_t));
                CreateSem(p->mutex, SEM_MUTEX_INITIALIZER);
            }
            ports[port_number]->initialized = 1;
        }
	}
	return 0;
}

//Send data from client to server and from server to client
int Send(int port_number, Message_t msg)
{
	PORT p = getPort(port_number);
	if(p == NULL)
		return -1;
	P(p->empty);
		P(p->mutex);
			p->q->mq[p->q->in] = (Message_t)malloc(sizeof(struct message));
			memcpy(p->q->mq[p->q->in], msg, sizeof(struct message));
			p->q->in = (p->q->in+1)%MESSAGE_Q_LENGTH;
			p->q->msg_count++;

			if(port_number==SERVER_PORT)
			{
			    printMsg(msg);
			}
		V(p->mutex);
	V(p->full);
	return 0;
}

//Receive method has been modified from previous version, to support strategy 3
int Receive(int port_number, Message_t msg)
{
	PORT p = getPort(port_number);
	if(p == NULL)
		return -1;
    while(1)
    {
        P(p->full);
		P(p->mutex);
			if(p->q->msg_count>0)
			{
                memcpy(msg, p->q->mq[p->q->out], sizeof(struct message));
                free(p->q->mq[p->q->out]);
                p->q->out = (p->q->out+1)%MESSAGE_Q_LENGTH;
                p->q->msg_count--;

                if(port_number!=SERVER_PORT)
                {
                    printf("\n[Client_on_port_%d]:Receive message:\t", port_number);
                    printMsg(msg);
                }

                V(p->mutex);
                V(p->empty);
                break;
			}
			else
			{
			    V(p->mutex);
			    V(p->empty);
			}
    }
	return 0;
}

//Deallocate memory of the ports
void closePort(int port_number)
{
    PORT p = getPort(port_number);
    if(p!=NULL)
    {
        free(p->q->mq);
        free(p->q);
        free(p);
    }
}

#endif
