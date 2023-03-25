#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AutoSocket.h"

#define MAXSTR 1024

void getMessage(char* buffer, size_t maxStr)
{
    bzero(buffer, maxStr);
    fgets(buffer, maxStr, stdin);
}

int main()
{
    const int port = 8090;
    const int proto = 0;
    int queueLength = 10;
    char ip[] = "67.190.23.178";
    char ip2[] = "192.168.1.151";
    char local_host[] = "127.0.0.1";

    struct AutoSocket sock;

    char* buffer = (char*)malloc(MAXSTR * sizeof(char));
    printf("Please Enter The Message You'd Like to Send:\n\t");
    getMessage(buffer, MAXSTR);

    createSocket(&sock, local_host, port, proto, queueLength);
    printf("[+] Socket Created!\n");

    connectToSocket(&sock);
    printf("[+] Socket Connected\n");

    sendMessageToSocket(&sock, buffer);
    printf("[+] Message Sent!\n");

    receiveMessageFromSocket(&sock, buffer, MAXSTR);
    printf("Message:\n\t%s\n\n", buffer);
    
    closeSocket(&sock);
    printf("[+] Socket closed!\n");
}