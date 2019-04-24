//Authors Alden and Saketh
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msgs.h"
#include <stdbool.h>

#define LOCK 100
#define UNLOCK 101
#define LOCKED 100
#define UNLOCKED 101
/*--------------------Server code---------------------------------*/
struct requestQ
{
	int * pending_queue;
	short in;
	short out;
};

struct table
{
	char *entries[10];
	int locker[10];
	short status[10];
	struct requestQ* q[10];
};

int Delete_Q(struct requestQ * a)
{
	int deleted;
	if(a->in == a->out)
		return -1;
	else
	{
		deleted = a->pending_queue[a->out];
		a->out = (a->out + 1) % 10;
	}

	return deleted;
}

int Add_Q(struct requestQ * a, int request)
{
	if((a->in + 1) % 10 == a->out)
		return -1;
	else
	{
		a->pending_queue[a->in] = request;
		a->in = (a->in + 1) % 10;
	}

	return request;
}

int Create_table(struct table* a)
{
	int i;
	a = malloc(sizeof(struct table));

	for(i = 0; i < 10; i++)
	{
		a->entries[i] = NULL;
		a->status[i] = UNLOCKED;
		a->q[i] = malloc(sizeof(struct requestQ));
		a->q[i]->in = 0;
		a->q[i]->out = 0;
	}

	return 1;
}

int Lock(struct table * a, int resource, int requestor)
{
	int result;
	if(a->status[resource] == LOCKED)
	{
		result = Add_Q(a->q[resource], requestor);
	}
	else
	{
		a->status[resource] = LOCKED;
		a->locker[resource] = requestor;
		result = LOCKED;
	}

	return result;
}

int Unlock(struct table * a, int resource)
{
	int result;
	if(a->status[resource] == LOCKED)
	{
		a->status[resource] = UNLOCKED;
		result = UNLOCKED;
	}
	else
	{
		result = -1;
	}

	return result;
}

int Add_Entry(struct table *a, int row, char * entry, bool first)
{
	int result = -1, i, len, ended = 0;

	if(a->entries[row] == NULL)
	{
		a->entries[row] = (char*)malloc(11*sizeof(char));
		for( i = 0; i < 10; i++)
		{
			a->entries[row][i] = entry[i];
			if(entry[i] == '\0') break;
		}
		if(entry[i] !='\0') a->entries[row][10] = '\0';
		result = 1;
	}
	else
	{
		if(!first)
		{
			len = strlen(a->entries[row]);
			a->entries[row] = (char*)realloc(a->entries[row], (11*sizeof(char)+strlen(a->entries[row])));
			for( i = 0; i < 10; i++)
			{
				a->entries[row][len+i] = entry[i];
				if(entry[i] == '\0')
				{
					ended = 1;
					break;
				}
			}
			if(!ended) a->entries[row][strlen(a->entries[row])] = '\0';

			result = 1;
		}
	}
	return result;
}

int Delete_Entry(struct table *a, int row)
{
	int result = -1;
	if(a->entries[row] != NULL)
	{
		//a->entries[row] = NULL;
        free(a->entries[row]);
		result = 1;
	}

	return result;
}

int Modify_Entry(struct table *a, int row, char * entry, bool first)
{
	int result = -1;

	if(a->entries[row] != NULL)
	{
		if(first)
		{
			Delete_Entry(a, row);
			Add_Entry(a, row, entry, true);
		}
		else
			Add_Entry(a, row, entry, false);

		result = 1;
	}

	return result;
}

void server()
{
    int i, j, run_length = 0, print_lock = 0;
    short print_Requested = 0;
    char * all_strings;
	struct table t;
	Create_table(&t);
	Message_t msg;
	msg = (Message_t)malloc(sizeof(struct message));

	openPort(SERVER_PORT);
	while(1)
	{
            print_lock = 0;
			for(i = 0; i < 10; i++)
				if(t.status[i] == LOCKED)
				{
					msg->error_code = -1;
					print_lock = 1;
				}

			if(print_Requested && !print_lock)
			{

				for(i = 0; i < 10; i++)
				{
				 if(t.entries[i]!=NULL)
                    run_length += strlen(t.entries[i]);
				}

				all_strings = malloc((run_length+10) * sizeof(char));
				run_length = 0;
				for(i = 0; i < 10; i++)
				{
					 if(t.entries[i]!=NULL)
					 {
					     for(j = 0; j < strlen(t.entries[i]); j++)
                            all_strings[run_length++] = t.entries[i][j];
						all_strings[run_length++] = '\n';
					 }

				}
				j=0;
				msg->total_size = run_length;
				msg->error_code = 1; //Extended transmission
				while((run_length = run_length - 10) > -9)
				{
					char line[MESSAGE_LENGTH] = "";
					for(i = 0; (i < MESSAGE_LENGTH) && (j < msg->total_size); i++)
						line[i] = all_strings[j++];
					memcpy(msg->msg, line, MESSAGE_LENGTH * sizeof(char));
					if(j >= msg->total_size)
						msg->error_code = 0; //End of transmission
					Send(msg->reply_port, msg);
				}
                print_lock = 0;
                print_Requested = 0;
			}


		Receive(SERVER_PORT, msg);

		//also, take care of print request
		if(msg->operation == PRINT)
		{
			//check if anything is locked
			for(i = 0; i < 10; i++)
				if(t.status[i] == LOCKED)
				{
					msg->error_code = -1;
					print_lock = 1;
					print_Requested = 1;
				}
			if(!print_lock)
			{
				for(i = 0; i < 10; i++)
					run_length += strlen(t.entries[i]);

				all_strings = malloc((run_length + 10) * sizeof(char));
				run_length = 0;
				for(i = 0; i < 10; i++)
				{
					for(j = 0; j < strlen(t.entries[i]); j++)
						all_strings[run_length++] = t.entries[i][j];
					all_strings[run_length++] = '\n';
				}
				j=0;
				msg->total_size = run_length;
				msg->error_code = 1; //Extended transmission
				while((run_length = run_length - 10) > -9)
				{
					char line[MESSAGE_LENGTH] = "";
					for(i = 0; (i < MESSAGE_LENGTH) && (j < msg->total_size); i++)
						line[i] = all_strings[j++];
					memcpy(msg->msg, line, MESSAGE_LENGTH * sizeof(char));
					if(j >= msg->total_size)
						msg->error_code = 0; //End of transmission
					Send(msg->reply_port, msg);
				}
			print_lock = 0;
			print_Requested = 0;
			}
			else
			{
			    continue;
			}

		}
		else if( t.status[msg->row] == LOCKED ) //Resource is locked
		{
			if(msg->reply_port != t.locker[msg->row]) //Resource is locked by some other client, cannot lock
				msg->error_code = -1;
			else //Resource requestor is also the resource locker
			{
				if(msg->error_code == 1) //Extended transmission ongoing, more msgs to come
				{
					if(msg->operation == ADD)
						msg->error_code = Add_Entry(&t, msg->row, msg->msg, false);
					else if(msg->operation == MODIFY)
						msg->error_code = Modify_Entry(&t, msg->row, msg->msg, false);
				}
				else //Last message of extended transmission
				{
					if(msg->operation == ADD)
						msg->error_code = Add_Entry(&t, msg->row, msg->msg, false);
					else if(msg->operation == MODIFY)
						msg->error_code = Modify_Entry(&t, msg->row, msg->msg, false);

					Unlock(&t, msg->row);
				}
			}
		}
		else //Not locked
		{
			if(msg->error_code == 1) //Extended transmission, lock
			{
				Lock(&t, msg->row, msg->reply_port);
				if(msg->operation == ADD)
					msg->error_code = Add_Entry(&t, msg->row, msg->msg, true);
				else if(msg->operation == MODIFY)
					msg->error_code = Modify_Entry(&t, msg->row, msg->msg, true);
			}
			else
			{
				if(msg->operation == ADD)
					msg->error_code = Add_Entry(&t, msg->row, msg->msg, true);
				else if(msg->operation == MODIFY)
					msg->error_code = Modify_Entry(&t, msg->row, msg->msg, true);
				else
					msg->error_code = Delete_Entry(&t, msg->row);

			}
		}
		Send(msg->reply_port, msg);

	}

	closePort(SERVER_PORT);
}

/*----------------------------------End of server code-----------------------*/

/**********************Client code*****************/

int createPacket(Message_t msg, int operation, int row, int packet_no)
{
	int i,code = 1; //Extended transmission
	msg->reply_port = getCurrentThreadId();
	msg->operation = operation;
	msg->row = row;
	char line[50] = "";
	char msg_line[MESSAGE_LENGTH] = "";
	char dig[2];
	dig[0] = (char)(((int)'0')+ msg->reply_port);
	dig[1] = '\0';
	if(operation == ADD)
	{
		strcat(line,"Added by client \0");
		strcat(line, dig);
	}
	else if(operation == MODIFY)
	{
		strcat(line,"Modified by client \0");
		strcat(line, dig);
	}
	else if(operation == DELETE)
	{
		strcat(line,"Deleted \0");
		dig[0] = (char)(((int)'0')+ msg->row);
		strcat(line, dig);
	}
    else
    {
        strcat(line, "PRINT\0");
    }

	for( i = 0; i < MESSAGE_LENGTH ; i++)
	{
		msg_line[i] = line[(packet_no * 10) + i];
		if(msg_line[i] == '\0')
		{
			code = 0; //End transmission
			break;
		}
	}
	memcpy(msg->msg, msg_line, MESSAGE_LENGTH * sizeof(char));
	msg->error_code = code;
	msg->total_size = strlen(line);
	return code;
}

void client_printer()
{
 	Message_t msg, reply;
    int reply_port, status, i, received = 0;
    char * all_strings;

    reply_port = getCurrentThreadId();

    openPort(reply_port);

    msg = (Message_t)malloc(sizeof(struct message));
    reply = (Message_t)malloc(sizeof(struct message));

    srand(getCurrentThreadId());

    while(1)
    {
        if(rand() % 3 == 0)
        {
            createPacket(msg, PRINT, 10, 0);

            printf("\n[Client_on_port_%d]:Sent message:\t", getCurrentThreadId());
            if((status = Send(SERVER_PORT, msg))<0)
            {
                     printf("\n[Client_%d]:Send error",getCurrentThreadId());
                     exit(-1);
            }

            if((status = Receive(reply_port, reply))<0)
            {
                printf("\n[Client_%d]:Receive error",getCurrentThreadId());
                exit(-1);
            }

            all_strings = malloc((reply->total_size) * sizeof(char));

            while(reply->error_code == 1)
            {
                for(i = 0; i < MESSAGE_LENGTH; i++)
                    all_strings[received++] = reply->msg[i];

                if((status = Receive(reply_port, reply))<0)
                {
                    printf("\n[Client_%d]:Receive error",getCurrentThreadId());
                    exit(-1);
                }

            }
			if(reply->error_code != -1)
			{
				i=0;
				while(received < reply->total_size)
					all_strings[received++] = reply->msg[i++];

				printf("Table Entries: \n %s", all_strings);
			}
        }
    }

}

void client()
{
	Message_t msg, reply;
	int reply_port, status, code, func, packet_no = 0;
	char precoded_string[15] = "+++--*+*-+--*-*";
	int op = 0;
	reply_port = getCurrentThreadId();

	openPort(reply_port);

	msg = (Message_t)malloc(sizeof(struct message));
    reply = (Message_t)malloc(sizeof(struct message));

    srand(getCurrentThreadId());

	while(1)
	{
		if(precoded_string[op] == '+')
			func = ADD;
		else if(precoded_string[op] == '-')
			func = DELETE;
		else
			func = MODIFY;

		do
		{
			code = createPacket(msg, func, rand() % 10, packet_no++);
			printf("\n[Client_on_port_%d]:Sent message:\t", getCurrentThreadId());
			if((status = Send(SERVER_PORT, msg))<0)
			{
                 printf("\n[Client_%d]:Send error",getCurrentThreadId());
                 exit(-1);
			}
			if((status = Receive(reply_port, reply))<0)
			{
				printf("\n[Client_%d]:Receive error",getCurrentThreadId());
				exit(-1);
			}
			if(reply->error_code == -1) break;
		}while(code);

		if(++op == 15)
			op=0;

		packet_no = 0;
		sleep(1);
	}
	closePort(reply_port);
}

/****************************End of client code****************************/

//This method instantiates 1 server and 2 clients, both the clients share the same semaphore
int main(int arc, char** argv)
{
	printf("\n Format of outputs: <client> <Received / Sent message> <content of packet> <code>");
	printf("\n Codes: -1 --> error  |  0 --> neutral  |  1 --> success \n");
	printf("Receive messages are ACKs");
	startThread(&server);
	startThread(&client);
	startThread(&client);
	startThread(&client_printer);
	run();
	return 0;
}
