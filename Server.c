#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AutoSocket.h"

#define MAXSTR 2048

int main()
{
    const int port = 8090;
    const int proto = 0;
    int queueLength = 10;
    char ip[] = "any";
    
    struct AutoSocket serverSock;
    struct AutoSocket clientSock;
    char* buffer = (char*)malloc(MAXSTR * sizeof(char)); bzero(buffer, MAXSTR);

    createSocket(&serverSock, ip, port, proto);
    serverSock.messageQueue = queueLength;
    printf("[+] Socket Created\n");

    bindSocketToPort(&serverSock);
    printf("[+] Socket Bound to port: %d\n", port);

    listenOnSocket(&serverSock);
    printf("[+] Listening for port: %d\n", port);

    while(true){
        printf("[ ] Waiting for Connection...\n");
        waitForConnection(&serverSock, &clientSock, buffer);

        clientSock.lineBufferLen = MAXSTR;

        receiveFromSocket(&clientSock, buffer);
        printf("[+] Received:\n\t%s\n", buffer);

        getSocketAddress(&clientSock, buffer);
        printf("[ ] FROM:\t%s\n", buffer);

        closeSocket(&clientSock);
    }
}