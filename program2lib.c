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
	printf("Amount of data sent is: %d\n", sent);
}

//function that counts number of words seperated by spaces in a string
int getWordCount(char * string){
    int wordCount = 0;
    int i = 0;
    while(1){
        if(string[i] == ' '){
            wordCount++;
        }
        if(string[i] == '\0'){
            wordCount++;
            break;
        }
        i++;
    }
    return wordCount;
}

//function that returns the number of words in a string and sets the pointers to the start of each word
//adds in null terminators to the string to seperate the words
int getWords(char * string, char ** startPtrs){
    int wordCount = 0;
    int i = 0;
    int start = 0;
    while(1){
        if((string[i] == ' ') || (string[i] == '\0')){
            string[i] = '\0';
            wordCount++;
            startPtrs[wordCount - 1] = string + start;
            start = i + 1;
            if (string[i] == '\0'){
                break;
            }
        }
        i++;
    }
    return wordCount;
}
