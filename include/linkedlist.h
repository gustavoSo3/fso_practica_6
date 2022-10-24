#include <stdlib.h>
#include <stdio.h>

struct Node
{
	int val;
	struct Node *next;
};

void addNode(struct Node *root, int val)
{
	struct Node *cur = root->next;
	struct Node *prev = root;
	while (cur != NULL && cur->val < val)
	{
		prev = cur;
		cur = cur->next;
	}
	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	node->val = val;
	node->next = cur;
	prev->next = node;
}

void delete (struct Node *node)
{
	struct Node *next;
	while (node != NULL)
	{
		next = node->next;
		free(node);
		node = next;
	}
}

void print(struct Node *node)
{
	while (node != NULL)
	{
		printf("%d ", node->val);
		node = node->next;
	}
	printf("\n");
}