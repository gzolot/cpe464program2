//include the basic c libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int getFlag(uint8_t *buffer){
    return buffer[2];
}

void sendPacket(uint8_t flag, uint8_t *buffer, int buffer_len, int socketNum){
	uint8_t sendBuf[buffer_len + 1];
	uint8_t *flag = sendBuf;
	*flag = flag;
	memcpy(sendBuf + 1, buffer, buffer_len);
	int sent = 0;
	sent = sendPDU(socketNum, sendbuf, len);
	if(sent < 0){
		perror("send call");
		exit(-1);
	}
	printf("Amount of data sent is: %d\n", sent);
}
