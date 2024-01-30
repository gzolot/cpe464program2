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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void processMsgFromServer(int socketNum);

// this function is used to handle a termination signal and set a global variable
// so that the program will terminate nicely

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	uint8_t handle_len = checkArgs(argc, argv);
	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

	setupConnection(handle_len, argv[1], socketNum);

	setupPollSet();
	addToPollSet(socketNum);
	addToPollSet(STDIN_FILENO);
	while(1){
		printf("Enter data: ");
		fflush(stdout);
		int pollReturn = pollCall(-1);
		if(pollReturn == STDIN_FILENO){
			sendToServer(socketNum);
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
	
	return 0;
}

void setupConnection(uint8_t handle_len, char *handle, int socketNum){
	sendInitialPacket(handle_len, handle, socketNum)
	//recieve buffer that is 3 bytes long
	uint8_t recvBuf[1];
	recvPDU(socketNum, recvBuf, 1);
	if(recvbuf[0] == 2){
		printf("Handle has been added to server\n")
	}
	else if (recvBuf == 3){
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
	if(recvLen == 0){
		printf("Server has terminated\n");
		exit(-1);
	}
	printf("Received: %s\n", recvBuf);
}

void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	sent =  sendPDU(socketNum, sendBuf, sendLen);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
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


