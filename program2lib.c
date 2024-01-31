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
