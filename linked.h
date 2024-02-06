//include all the necessary libraries for a linked list
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct handle
{
    char *name;
    int socketNumber;
    struct handle *next;
};

void addNode(int socketNumber, char *name, int nameLength);
int getSocketNumber(char *name);
void removeNode(int socketNumber);
void printList(void);
uint32_t getLength(void);
void getAllHandles(char **listOfHandles);



