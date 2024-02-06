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
void errorPacket(int clientSocket, char *handle, int handle_len);
void sendFlag11(int clientSocket);

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
		close(clientSocket);
		removeFromPollSet(clientSocket);
		removeNode(clientSocket);
	}

	if (messageLen > 0)
	{
		if(dataBuffer[0] == 1){
			initializeClient(clientSocket, dataBuffer);
		}
		else if(dataBuffer[0] == 5 || dataBuffer[0] == 6){
			processMessage(clientSocket, dataBuffer, messageLen);
		}
		else if(dataBuffer[0] == 10){
			sendFlag11(clientSocket);
		}
		else if(dataBuffer[0] == 8){
			sendPacket(9, NULL, 0, clientSocket);
			removeNode(clientSocket);
			//printList();
			close(clientSocket);
			removeFromPollSet(clientSocket);
		}
		else{
			printf("Error in packet\n");
		}
		//printf("Message received, Socket: %d length: %d Data: %s\n",clientSocket, messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
		close(clientSocket);
		removeFromPollSet(clientSocket);
		removeNode(clientSocket);
	}
}

void sendFlag11(int clientSocket){
	uint32_t lengthHost = getLength();
	char *listOfHandles[lengthHost];
	uint32_t lengthNetwork = htonl(lengthHost);
	uint8_t sendBuf[4];
	memcpy(sendBuf, &lengthNetwork, 4);
	sendPacket(11, sendBuf, 4, clientSocket);
	getAllHandles(listOfHandles);
	while(lengthHost > 0){
		uint8_t handle_len = strlen(listOfHandles[lengthHost-1]);
		uint8_t sendBuf[handle_len + 1];
		memcpy(sendBuf, &handle_len, 1);
		memcpy(sendBuf + 1, listOfHandles[lengthHost-1], handle_len);
		sendPacket(12, sendBuf, handle_len + 1, clientSocket);
		lengthHost--;
	}
}

void processMessage(int clientSocket, uint8_t *dataBuffer, int messageLen){
	uint8_t sendingHandleLen = dataBuffer[1];
	int bufferOffset = 1+1+sendingHandleLen;
	uint8_t numberOfHandles = dataBuffer[bufferOffset];
	//printf("numberOfHandles: %d\n", numberOfHandles);
	bufferOffset++;
	int handleCounter = 0;
	while(handleCounter < numberOfHandles){
		uint8_t destHandleLen = dataBuffer[bufferOffset];
		bufferOffset++;
		char destHandle[destHandleLen+1];
		//printf("destHandleLen: %d\n", destHandleLen);
		memcpy(destHandle, dataBuffer + bufferOffset, destHandleLen);
		bufferOffset += destHandleLen;
		destHandle[destHandleLen] = '\0';
		int destSocket = getSocketNumber(destHandle);
		//printf("desthandle: %s\n", destHandle);
		if(destSocket != -1){
			sendPDU(destSocket, dataBuffer, messageLen);
		}
		else{
			printf("sending error packet with handle name %s and length%d\n", destHandle, destHandleLen);
			errorPacket(clientSocket, destHandle, destHandleLen);
		}
		handleCounter++;
	}

}

void errorPacket(int clientSocket, char *handle, int handle_len){
	uint8_t sendBuf[handle_len + 1];
	memcpy(sendBuf, &handle_len, 1);
	memcpy(sendBuf + 1, handle, handle_len);
	sendPacket(7, sendBuf, handle_len + 1, clientSocket);
}

void initializeClient(int clientSocket, uint8_t *dataBuffer){
	uint8_t handle_len = dataBuffer[1];
	char handle[handle_len+1];
	memcpy(handle, dataBuffer + 2, handle_len);
	handle[handle_len] = '\0';
	if(getSocketNumber(handle) == -1){
		addNode(clientSocket, handle, handle_len+1);
		//printList();
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

