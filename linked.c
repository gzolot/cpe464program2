#include "linked.h"

struct handle *head = NULL;

void addNode(int socketNumber, char *name, int nameLength){
    struct handle *newNode = (struct handle *)malloc(sizeof(struct handle));
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
    struct handle *current = head;
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
    struct handle *current = head;
    while (current != NULL){
        if (strcmp(current->name, name) == 0){
            return current->socketNumber;
        }
        current = current->next;
    }
    return -1;
}

void removeNode(int socketNumber){
    struct handle *current = head;
    struct handle *prev = NULL;
    while (current != NULL){
        if (current->socketNumber == socketNumber){
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
    struct handle *current = head;
    while (current != NULL){
        printf("Socket Number: %d, Name: %s\n", current->socketNumber, current->name);
        current = current->next;
    }
}

void getAllHandles(char **listOfHandles){
    struct handle *current = head;
    int i = 0;
    while (current != NULL){
        listOfHandles[i] = current->name;
        current = current->next;
        i++;
    }
}

uint32_t getLength(){
    uint32_t length = 0;
    struct handle *current = head;
    while (current != NULL){
        length++;
        current = current->next;
    }
    return length;
}