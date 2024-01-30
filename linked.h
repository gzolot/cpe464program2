//include all the necessary libraries for a linked list
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct handle
{
    char *name;
    int socket_num;
    struct handle *next;
}

