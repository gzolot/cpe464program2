/******************************************************************************
* server.c
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
#include "linked.h"


#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CLIENTS 10

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int mainServerSocket);
void addNewSocket(int mainServerSocket);
void processClient(int clientSocket);
void initializeClient(int clientSocket, uint8_t *dataBuffer);
void processMessage(int clientSocket, uint8_t *dataBuffer, int messageLen);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);
	
	serverControl(mainServerSocket);
	
	/* close the sockets */
	close(clientSocket);
	close(mainServerSocket);

	
	return 0;
}

void serverControl(int mainServerSocket){
	setupPollSet();
	addToPollSet(mainServerSocket);
	while(1){
		int pollReturn = pollCall(-1);
		if (pollReturn < 0)
		{
			perror("poll call");
			exit(-1);
		}
		if (pollReturn == mainServerSocket){
			addNewSocket(mainServerSocket);
		}
		else{
			processClient(pollReturn);
		}
	}
}

void addNewSocket(int mainServerSocket){
	int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	addToPollSet(clientSocket);
}

void processClient(int clientSocket){
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
		close(clientSocket);
		removeFromPollSet(clientSocket);
	}

	if (messageLen > 0)
	{
		if(dataBuffer[0] == 1){
			initializeClient(clientSocket, dataBuffer);
		}
		if(dataBuffer[0] == 5){
			processMessage(clientSocket, dataBuffer, messageLen);
		}
		printf("Message received, Socket: %d length: %d Data: %s\n",clientSocket, messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
		close(clientSocket);
		removeFromPollSet(clientSocket);
	}
}

void processMessage(int clientSocket, uint8_t *dataBuffer, int messageLen){
	uint8_t sendingHandleLen = dataBuffer[1];
	int bufferOffset = 1+1+sendingHandleLen;
	uint8_t numberOfHandles = dataBuffer[bufferOffset];
	bufferOffset++;
	if(numberOfHandles ==1){
		uint8_t destHandleLen = dataBuffer[bufferOffset];
		bufferOffset++;
		char desHandle[destHandleLen];
		memcpy(desHandle, dataBuffer + bufferOffset, destHandleLen);
		int destSocket = getSocketNumber(desHandle);
		if(destSocket != -1){
			sendPDU(destSocket, dataBuffer, messageLen);
		}
		else{
			printf("destination handle %s not found\n", desHandle);
		}
	}

}

void initializeClient(int clientSocket, uint8_t *dataBuffer){
	uint8_t handle_len = dataBuffer[1];
	char handle[handle_len+1];
	memcpy(handle, dataBuffer + 2, handle_len);
	handle[handle_len] = '\0';
	if(getSocketNumber(handle) == -1){
		addNode(clientSocket, handle, handle_len+1);
		printList();
		sendPacket(2, NULL, 0, clientSocket);
	}
	else{
		sendPacket(3, NULL, 0, clientSocket);
	}
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Message received, length: %d Data: %s\n", messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

