/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "pdu.h"
#include "pollLib.h"
#include "program2lib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAXMSGSIZE 199

void sendToServer(int socketNum, char * handle, uint8_t handle_len);
int readFromStdin(uint8_t * buffer);
uint8_t checkArgs(int argc, char * argv[]);
void processMsgFromServer(int socketNum);
void setupConnection(uint8_t handle_len, char *handle, int socketNum);
void sendInitialPacket(uint8_t handle_len, char * handle, int socketNum);
void sendMessage(int socketNum, char ** startPtrs, char * handle, uint8_t handle_len, uint8_t recieverCount);
void printMessagePacket(uint8_t * recvBuf);
void printErrorPacket(uint8_t * recvBuf);
void sendBroadcast(int socketNum, char * handle, uint8_t handle_len, char * message);
void clientControl(int socketNum, uint8_t handle_len, char * handle);
void sendextraLongMessage(int socketNum, uint8_t *sendBuf, int bufferOffset, char *message, int messagelen, uint8_t flag);

// this function is used to handle a termination signal and set a global variable
// so that the program will terminate nicely

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	uint8_t handle_len = checkArgs(argc, argv);
	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

	setupConnection(handle_len, argv[1], socketNum);
	clientControl(socketNum, handle_len, argv[1]);
	return 0;
}

void clientControl(int socketNum, uint8_t handle_len, char * handle){
	setupPollSet();
	addToPollSet(socketNum);
	addToPollSet(STDIN_FILENO);
	while(1){
		printf("$: ");
		fflush(stdout);
		int pollReturn = pollCall(-1);
		if(pollReturn == STDIN_FILENO){
			sendToServer(socketNum, handle, handle_len);
		}
		else if(pollReturn == socketNum){
			processMsgFromServer(socketNum);
		}
		else{
			printf("Error in poll return\n");
			exit(-1);
		}
	}
	close(socketNum);
}

void setupConnection(uint8_t handle_len, char *handle, int socketNum){
	sendInitialPacket(handle_len, handle, socketNum);
	//recieve buffer that is 3 bytes long
	uint8_t recvBuf[1];
	recvPDU(socketNum, recvBuf, 1);
	if(recvBuf[0] == 2){
		printf("Handle has been added to server\n");
	}
	else if (recvBuf[0] == 3){
		printf("Handle already exists\n");
		exit(-1);
	}
	else{
		printf("Error in initial packet\n");
		exit(-1);
	}

}

void sendInitialPacket(uint8_t handle_len, char * handle, int socketNum){
	uint8_t sendBuf[handle_len + 1];
	uint8_t *length = sendBuf;
	*length = handle_len;
	memcpy(sendBuf + 1, handle, handle_len);
	sendPacket(1, sendBuf, handle_len + 1, socketNum);

}

void processMsgFromServer(int socketNum){
	uint8_t recvBuf[MAXBUF];
	int recvLen = recvPDU(socketNum, recvBuf, MAXBUF);
	uint8_t flag = recvBuf[0];
	if(recvLen == 0){
		printf("Server has terminated\n");
		exit(-1);
	}
	if (flag == 5 || flag == 6){
		printMessagePacket(recvBuf);
	}
	else if(flag == 7){
		printErrorPacket(recvBuf);
	}
	else if(flag == 11){
		uint32_t *elements = (uint32_t *) &recvBuf[1];
		uint32_t clients = ntohl(*elements);
		printf("\nNumber of clients: %d\n", clients);
	}
	else if(flag == 12){
		uint8_t handleLen = recvBuf[1];
		char handle[handleLen+1];
		memcpy(handle, recvBuf + 2, handleLen);
		handle[handleLen] = '\0';
		printf("\t%s\n", handle);
	}
	else if(flag == 9){
		exit(0);
	}
	else if(flag == 4){
		uint8_t handleLen = recvBuf[1];
		char handle[handleLen+1];
		memcpy(handle, recvBuf + 2, handleLen);
		handle[handleLen] = '\0';
		printf("\n%s: %s\n", handle, recvBuf + 2 + handleLen);
	}
	else{
		printf("Error in packet\n");
		exit(-1);
	}
}


void printMessagePacket(uint8_t * recvBuf){
	uint8_t sendClientHandlelen = recvBuf[1];
	char sendClientHandle[sendClientHandlelen+1];
	memcpy(sendClientHandle, recvBuf + 2, sendClientHandlelen);
	sendClientHandle[sendClientHandlelen] = '\0';
	uint8_t numofRecervers = recvBuf[2 + sendClientHandlelen];
	uint8_t counter = 0;
	int bufferOffset = 3 + sendClientHandlelen;
	while(counter < numofRecervers){
		uint8_t destHandleLen = recvBuf[bufferOffset];
		bufferOffset += 1+destHandleLen;
		counter++;
	}
	char *message = (char *)recvBuf + bufferOffset;
	//printf("in print message packet\n");
	printf("\n%s: %s\n", sendClientHandle, message);
}

void printErrorPacket(uint8_t * recvBuf){
	uint8_t handleLen = recvBuf[1];
	char handle[handleLen+1];
	memcpy(handle, recvBuf + 2, handleLen);
	handle[handleLen] = '\0';
	printf("Client with handle %s does not exist.\n", handle);
}

void sendToServer(int socketNum, char * handle, uint8_t handle_len)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	// int sendLen = 0;        //amount of data to send
	// int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	// char *inputStr = (char *)sendBuf;
	readFromStdin(sendBuf);


	int wordCount = getWordCount((char *) sendBuf);
	if(wordCount == 0){
		printf("Invalid command\n");
		return;
	}
	int startIndexSecondWrd = getFirst((char *)sendBuf);
	char *firstWord = (char *)sendBuf;

	if (!(strcmp(firstWord, "%M")) || !(strcmp(firstWord, "%m"))){
		//process message packet
		if(wordCount < 2){
			printf("Invalid message\n");
			return;
		}
		char *startPtrs[2];
		getWords((char *)sendBuf+startIndexSecondWrd, 1, startPtrs);
		//printf("first word: %s\nsecond word: %s\n", startPtrs[0], startPtrs[1]);
		//printf("Message packet\n");
		sendMessage(socketNum, startPtrs, handle, handle_len, 1);

	}
	else if(!(strcmp(firstWord, "%B")) || !(strcmp(firstWord, "%b"))){
		//process broadcast packet
		if(wordCount < 2){
			printf("Invalid message\n");
			return;
		}
		char *message = (char *)sendBuf+startIndexSecondWrd;
		sendBroadcast(socketNum, handle, handle_len, message);
		//printf("Broadcast packet\n");

	}
	else if(!(strcmp(firstWord, "%L")) || !(strcmp(firstWord, "%l"))){
		//process list packet
		//printf("List packet\n");
		sendPacket(10, NULL, 0, socketNum);
	}
	else if(!(strcmp(firstWord, "%E")) || !(strcmp(firstWord, "%e"))){
		//process exit packet
		//printf("Exit packet\n");
		sendPacket(8, NULL, 0, socketNum);
	}
	else if(!(strcmp(firstWord, "%C")) || !(strcmp(firstWord, "%c"))){
		//process multicast packet
		//printf("Multicast packet\n");
		if (wordCount < 3){
			printf("Invalid message\n");
			return;
		}
		char *numHandles[2];
		getWords((char *)sendBuf+startIndexSecondWrd, 1, numHandles);
		int handleCount = atoi(numHandles[0]);
		if(handleCount < 2 || handleCount > 9){
			printf("There should be 2 to 9 recieving handles with a multicast\n");
			return;
		}
		char *destHandleNames[handleCount+1];
		getWords(numHandles[1], handleCount, destHandleNames);
		sendMessage(socketNum, destHandleNames, handle, handle_len, handleCount);
	// if (handleCount)
		//print all the elements in destHandleNames
		// for(int i = 0; i < handleCount; i++){
		// 	printf("Handle: %s\n", destHandleNames[i]);
		// }
		// printf("Message: %s\n", destHandleNames[handleCount]);


	}
	else{
		printf("Invalid command\n");
		return;
	}


	// sent =  sendPDU(socketNum, sendBuf, sendLen);
	// if (sent < 0)
	// {
	// 	perror("send call");
	// 	exit(-1);
	// }

	// printf("Amount of data sent is: %d\n", sent);
}

void sendBroadcast(int socketNum, char * handle, uint8_t handle_len, char * message){
	uint8_t sendBuf[MAXBUF];
	memcpy(sendBuf, &handle_len, 1);
	memcpy(sendBuf + 1, handle, handle_len);
	//int bufferOffset = 2 + handle_len + strlen(message);
	int messageLen = strlen(message);
	if (messageLen > MAXMSGSIZE){
		sendextraLongMessage(socketNum, sendBuf, 1 + handle_len, message, messageLen, 4);
	}
	else{
		memcpy(sendBuf + 1 + handle_len, message, strlen(message)+1);
		sendPacket(4, sendBuf, handle_len + strlen(message) + 2, socketNum);
	}
}

void sendMessage(int socketNum, char ** startPtrs, char * handle, uint8_t handle_len, uint8_t recieverCount){
	uint8_t sendBuf[MAXBUF];
	uint8_t flag;
	if (recieverCount > 1){
		flag = 6;
	}
	else{
		flag = 5;
	}
	memcpy(sendBuf, &handle_len, 1);
	memcpy(sendBuf + 1, handle, handle_len);
	memcpy(sendBuf + 1 + handle_len, &recieverCount, 1);
	int handleCount = 0;
	int bufferOffset = 2 + handle_len;
	while(handleCount < recieverCount){
		uint8_t destHandleLen = strlen(startPtrs[handleCount]);
		memcpy(sendBuf + bufferOffset, &destHandleLen, 1);
		bufferOffset++;
		memcpy(sendBuf + bufferOffset, startPtrs[handleCount], destHandleLen);
		bufferOffset += destHandleLen;
		handleCount++;
	}
	if (startPtrs[handleCount] != NULL){
		int messagelen = strlen(startPtrs[handleCount]);
		//printf("Message length: %d\n", messagelen);
		if (messagelen > MAXMSGSIZE){
			//printf("Message is too long\n");
			sendextraLongMessage(socketNum, sendBuf, bufferOffset, startPtrs[handleCount], messagelen, flag);
		}
		else{
			memcpy(sendBuf + bufferOffset, startPtrs[handleCount], strlen(startPtrs[handleCount])+1);
			bufferOffset += strlen(startPtrs[handleCount])+1;
			sendPacket(flag, sendBuf, bufferOffset, socketNum);
		}
	}
	else{
		char *nullstring = "\n\0";
		memcpy(sendBuf + bufferOffset, nullstring, 2);
		bufferOffset += 2;
		sendPacket(flag, sendBuf, bufferOffset, socketNum);
	}
	// uint8_t destHandleLen = strlen(startPtrs[0]);
	// memcpy(sendBuf + 2 + handle_len, &destHandleLen, 1);
	// memcpy(sendBuf + 3 + handle_len, startPtrs[0], strlen(startPtrs[0]));
	// if (startPtrs[1] != NULL){
	// 	memcpy(sendBuf + 3 + handle_len + strlen(startPtrs[0]), startPtrs[1], strlen(startPtrs[1])+1);
	// }
	// else{
	// 	char *nullstring = "\n\0";
	// 	memcpy(sendBuf + 3 + handle_len + strlen(startPtrs[0]), nullstring, 2);
	// }
	//printf("The following message was sent to the server: %s\n", startPtrs[1]);
}

void sendextraLongMessage(int socketNum, uint8_t *sendBuf, int bufferOffset, char *message, int messagelen, uint8_t flag){
	int messageOffset = 0;
	while(messagelen > MAXMSGSIZE){
		memcpy(sendBuf + bufferOffset, message + messageOffset, MAXMSGSIZE);
		memcpy(sendBuf + bufferOffset + MAXMSGSIZE, "\0", 1);
		messagelen -= MAXMSGSIZE;
		messageOffset += MAXMSGSIZE;
		sendPacket(flag, sendBuf, MAXMSGSIZE + bufferOffset + 1, socketNum);
		//printf("message sent is %s\n", sendBuf + bufferOffset);
	}
	memcpy(sendBuf + bufferOffset, message + messageOffset, messagelen);
	memcpy(sendBuf + bufferOffset + messagelen, "\0", 1);
	sendPacket(flag, sendBuf, messagelen + bufferOffset + 1, socketNum);
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	// printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

uint8_t checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s handle server-name server-port\n", argv[0]);
		exit(1);
	}
	//check if handle is 100 characters or less
	if(strlen(argv[1]) > 100){
		printf("Handle must be 100 characters or less\n");
		exit(-1);
	}
	// return the length of the handle
	return strlen(argv[1]);
}


