#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "linkedlist.h"

#define NUM_PRODUCERS 4

typedef struct MsgBuffer
{
	long type;
	int content;
} Message;

void producer(int start, int end, int msg);
void consumer(int msg);
int isprime(int n);

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		return 0;
	}
	// Set the interval
	int start = atoi(argv[1]);
	int end = atoi(argv[2]);

	if (start < 2)
		start = 2;

	key_t key = ftok("progfile", 65);
	int msgid = msgget(key, 0666 | IPC_CREAT);

	int i;
	int pstart = start;
	if (pstart % 2 == 0 && pstart != 2)
		pstart += 1;

	for (i = 0; i < NUM_PRODUCERS; i++)
	{
		if (!fork())
		{
			// if child create producer
			producer(pstart, end, msgid);
		}
		pstart += pstart == 2 ? 3 : 2;
	}
	if (!fork())
	{
		// if child create consumer
		consumer(msgid);
	}

	for (i = 0; i < NUM_PRODUCERS + 1; i++)
		wait(NULL);

	// eliminar la cola de mensajes
	msgctl(msgid, IPC_RMID, NULL);

	return 0;
}

void send(int value, int msgid)
{
	Message msg;
	msg.type = 1;
	msg.content = value;
	msgsnd(msgid, &msg, sizeof(msg), 0);
}

void producer(int start, int end, int msgid)
{
	if (start == 2 && start < end)
	{
		send(2, msgid);
		start += 1;
	}

	for (int i = start; i < end; i += (2 * NUM_PRODUCERS))
	{
		if (isprime(i))
		{
			send(i, msgid);
		}
	}

	// mandar un mensaje para que el consumidor sepa que ya termino su trabajo
	send(-1, msgid);
	exit(0);
}

void consumer(int msgid)
{
	int finished = 0;
	struct Node root;
	root.val = 0;
	root.next = NULL;
	Message msg;
	while (finished != NUM_PRODUCERS)
	{
		msgrcv(msgid, &msg, sizeof(msg), 1, 0);
		if (msg.content == -1)
			finished++;
		else
			addNode(&root, msg.content);
	}
	print(root.next);
	delete (root.next);
	exit(0);
}

int isprime(int n)
{
	int d = 3;
	int prime = 0;
	int limit = sqrt(n);

	if (n < 2)
		prime = 0;
	else if (n == 2)
		prime = 1;
	else if (n % 2 == 0)
		prime = 0;
	else
	{
		while (d <= limit && n % d)
			d += 2;
		prime = d > limit;
	}
	return (prime);
}
