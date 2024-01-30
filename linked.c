#include "linked.h"

struct node *head = NULL;

void addNode(int socketNumber, char *name, int nameLength){
    struct node *newNode = (struct node *)malloc(sizeof(struct node));
    if (newNode == NULL){
        perror("malloc");
        exit(-1);
    }
    newNode->socketNumber = socketNumber;
    newNode->name = (char *)malloc(nameLength);
    if (newNode->name == NULL){
        perror("malloc");
        exit(-1);
    }
    strcpy(newNode->name, name);
    struct node *current = head;
    while (current != NULL){
        if (current->next == NULL){
            current->next = newNode;
            newNode->next = NULL;
            return;
        }
        current = current->next;
    }
    head = newNode;
    newNode->next = NULL;
}

int getSocketNumber(char *name){
    struct node *current = head;
    while (current != NULL){
        if (strcmp(current->name, name) == 0){
            return current->socketNumber;
        }
        current = current->next;
    }
    return -1;
}

void removeNode(char *name){
    struct node *current = head;
    struct node *prev = NULL;
    while (current != NULL){
        if (strcmp(current->next, name) == 0){
            if (prev == NULL){
                head = current->next;
            }
            else{
                prev->next = current->next;
            }
            free(current->name);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void printList(){
    struct node *current = head;
    while (current != NULL){
        printf("Socket Number: %d, Name: %s\n", current->socketNumber, current->name);
        current = current->next;
    }
}

int getLength(){
    int length = 0;
    struct node *current = head;
    while (current != NULL){
        length++;
        current = current->next;
    }
    return length;
}