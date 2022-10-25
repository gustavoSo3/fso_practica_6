#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "semaphoresarr.h"
#include "linkedlist.h"

#define BUFFER_SIZE 200
#define NUM_PRODUCERS 4

struct BUFFER
{
	int add;
	int read;
	int arr[BUFFER_SIZE];
	int pended; // Shows how many producers have ended.
};

struct BUFFER *buffer;

SEM_ID semarr;
enum
{
	B_FULL,
	B_EMPTY,
	S_EXMUT
};

void producer(int start, int end);
void add(int i);
void consumer();
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

	// Create the shared memory
	int shmid;

	srand(getpid());

	shmid = shmget((key_t)0x1234, sizeof(struct BUFFER), 0666 | IPC_CREAT);
	if (shmid == -1)
		exit(1);
	semwait(semarr, S_EXMUT); // See if critic section is on use

	// Set the buffer and init it
	buffer = shmat(shmid, NULL, 0);
	buffer->add = 0;
	buffer->read = 0;
	buffer->pended = 0;

	// Create semaphores
	semarr = createsemarray((key_t)9234, 3);

	initsem(semarr, B_FULL, BUFFER_SIZE);
	initsem(semarr, B_EMPTY, 0);
	initsem(semarr, S_EXMUT, 1);

	int i;
	int pstart = start;
	if (pstart % 2 == 0 && pstart != 2)
		pstart += 1;

	for (i = 0; i < NUM_PRODUCERS; i++)
	{
		if (!fork())
		{
			// if child create producer
			producer(pstart, end);
		}
		pstart += pstart == 2 ? 3 : 2;
	}
	if (!fork())
	{
		// if child create consumer
		consumer();
	}

	for (i = 0; i < NUM_PRODUCERS + 1; i++)
		wait(NULL);
	erasesem(semarr, B_FULL);
	erasesem(semarr, B_EMPTY);
	erasesem(semarr, S_EXMUT);
	shmdt(buffer);
	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}

void producer(int start, int end)
{

	if (start == 2 && start < end)
	{
		add(2);
		start += 1;
	}

	for (int i = start; i < end; i += (2 * NUM_PRODUCERS))
	{
		if (isprime(i))
		{
			add(i);
		}
	}
	semwait(semarr, S_EXMUT); // wait CS
	buffer->pended += 1;
	if (buffer->pended == NUM_PRODUCERS)
		semsignal(semarr, B_EMPTY);

	semsignal(semarr, S_EXMUT); // signal CS

	exit(0);
}

void add(int i)
{
	semwait(semarr, B_FULL);	// wait if buffer is full
	semwait(semarr, S_EXMUT); // wait if critic section is on use

	buffer->arr[buffer->add] = i;
	buffer->add = (buffer->add + 1) % BUFFER_SIZE;

	semsignal(semarr, S_EXMUT); // signal critical section free
	semsignal(semarr, B_EMPTY); // signal not empty buffer
}

void consumer()
{
	int end = 0;
	int val;

	struct Node root;
	root.val = 0;
	root.next = NULL;

	while (!end)
	{
		semwait(semarr, B_EMPTY); // wait if empty buffer
		semwait(semarr, S_EXMUT); // wait if critic section is on use

		if (buffer->read == buffer->add && buffer->pended == NUM_PRODUCERS)
			end = 1;
		else
		{
			val = buffer->arr[buffer->read];
			buffer->read = (buffer->read + 1) % BUFFER_SIZE;
		}

		semsignal(semarr, S_EXMUT);
		semsignal(semarr, B_FULL);
		if (!end)
			addNode(&root, val);
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
