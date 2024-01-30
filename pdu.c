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

#include "networks.h"
#include "safeUtil.h"

int sendPDU(int clientSocket, uint8_t *databuffer, int lengthofdata){
    //create a pdu with a two byte length field procedeeding the data
    uint8_t pdu[lengthofdata + 2];
    uint16_t *length = (uint16_t *) pdu;
    *length = htons((uint16_t)lengthofdata + 2);
    memcpy(pdu + 2, databuffer, lengthofdata);

    //send the pdu
    int sent = safeSend(clientSocket, pdu, lengthofdata + 2, 0);
    return sent -2;
}

int recvPDU(int clientSocket, uint8_t *databuffer, int bufSize){
    //receive the pdu
    uint8_t pdu[2];
    int received = safeRecv(clientSocket, pdu, 2, MSG_WAITALL);
    if(received == 0){
        return 0;
    }
    uint16_t *length = (uint16_t *) pdu;
    *length = ntohs(*length);
    //printf("bufSize: %d, length: %d\n", bufSize, *length);
    if (bufSize < *length - 2){
        printf("Buffer size is too small to receive the pdu\n");
        exit(-1);
    }
    received = safeRecv(clientSocket, databuffer, *length - 2, MSG_WAITALL);
    return received;
}