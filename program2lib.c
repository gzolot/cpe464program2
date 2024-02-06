//include the basic c libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>
#include "pdu.h"

void sendPacket(uint8_t flag, uint8_t *buffer, int buffer_len, int socketNum){
	int length = 0;
    if (buffer_len == 0){
        length = 1;
    }
    else{
        length = buffer_len + 1;
    }
    uint8_t sendBuf[length];
    sendBuf[0] = flag;
    if(buffer_len != 0){
	    memcpy(sendBuf + 1, buffer, buffer_len);
    }
	int sent = 0;
	sent = sendPDU(socketNum, sendBuf, length);
	if(sent < 0){
		perror("send call");
		exit(-1);
	}
	//printf("Amount of data sent is: %d\n", sent);
}

//function that counts number of words seperated by spaces in a string
int getWordCount(char * string){
    int wordCount = 0;
    int i = 0;
    char prevChar = ' ';
    while(1){
        if((string[i] == ' ') && (prevChar != ' ')){
            wordCount++;
        }
        if(string[i] == '\0'){
            wordCount++;
            break;
        }
        prevChar = string[i];
        i++;
    }
    return wordCount;
}

int getFirst(char * string){
    int i = 0;
    while(1){
        if(string[i] == ' '){
            string[i] = '\0';
            return i+1;
        }
        if(string[i] == '\0'){
            return -1;   
        }
        i++;
    }
}

//function that returns the number of words in a string and sets the pointers to the start of each word
//adds in null terminators to the string to seperate the words
void getWords(char * string, int numWords, char ** startPtrs){
    int wordCount = 0;
    int i = 0;
    int start = 0;
    char lastChar = ' ';
    while(1){
        if(((string[i] == ' ') || (string[i] == '\0')) && (lastChar != ' ')){
            wordCount++;
            startPtrs[wordCount - 1] = string + start;
            start = i + 1;
            if(numWords == wordCount){
                if(string[i] == '\0'){
                    startPtrs[wordCount] = NULL;
                    return;
                }
                string[i] = '\0';
                startPtrs[wordCount] = string + start;
                return;
            }
            else if (string[i] == '\0'){
                return;
            }
            string[i] = '\0';
        }
        lastChar = string[i];
        i++;
    }
}
