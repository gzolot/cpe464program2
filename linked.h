//include all the necessary libraries for a linked list
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct handle
{
    char *name;
    int socketNumber;
    struct handle *next;
};

void addNode(int socketNumber, char *name, int nameLength);
int getSocketNumber(char *name);
void removeNode(char *name);
void printList(void);
int getLength(void);



