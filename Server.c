#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AutoSocket.h"

#define MAXSTR 1024

int main()
{
    const int port = 8090;
    const int proto = 0;
    int queueLength = 10;
    char ip[] = "any";
    
    struct AutoSocket serverSock;
    struct AutoSocket clientSock;
    char* buffer = (char*)malloc(MAXSTR * sizeof(char)); bzero(buffer, MAXSTR);

    createSocket(&serverSock, ip, port, proto, queueLength);
    printf("[+] Socket Created\n");

    bindSocketToPort(&serverSock);
    printf("[+] Socket Bound to port: %d\n", port);

    listenOnSocket(&serverSock);
    printf("[+] Listening for port: %d\n", port);

    while(!strncmp(buffer, "EOL", 3) == 0){
        printf("[ ] Waiting for Connection...\n");
        waitForConnection(&serverSock, &clientSock, buffer, MAXSTR);
        receiveMessageFromSocket(&clientSock, buffer, MAXSTR);
        printf("[+] Received:\n\t%s\n", buffer);
        printf("[ ] FROM:\t%s\n", getSocketAddress(&clientSock));

        if(strcmp(buffer, "FILE REQUEST") == 0){
            printf("[ ]\tFile Sent!");
        }
        closeSocket(&clientSock);
    }

    closeSocket(&serverSock);
    printf("\n<--END!-->\n\n");
}