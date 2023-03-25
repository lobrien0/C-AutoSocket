#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <time.h>
#include <dirent.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef AUTO_SOCKET_H
#define AUTO_SOCKET_H

struct AutoSocket {
    struct sockaddr_in sock;
    int messageQueue;
    int sockfd;
    short proto; 
};

void createSocket(struct AutoSocket*, char*, const int, short, int);
void connectToSocket(struct AutoSocket*);
void bindSocketToPort(struct AutoSocket*);
int sendMessageToSocket(struct AutoSocket*, char*);
void receiveMessageFromSocket(struct AutoSocket*, char*, size_t);
void sendFileList(struct AutoSocket*);
void getMessageTime(char*, size_t);
char* getSocketAddress(struct AutoSocket*);
void listenOnSocket(struct AutoSocket*);
void waitForConnection(struct AutoSocket*, struct AutoSocket*, char*, size_t);
void sendFileOverSocket(char* fname, struct AutoSocket*);
void receiveFileOverSocket(struct AutoSocket*);
bool socketActive(struct AutoSocket*);
void closeSocket(struct AutoSocket*);

/*  Create Socket
        Using the structure AutoSock the program will first set all the parameters into their 
        proper structures before creating and storing the resulting entity.
        We set the IP, Port, Protocol, and Length of Backup Queue (only used for server-mode)
*/
void createSocket(struct AutoSocket* addr, char* ip, const int port, short protos, int backupQueue)
{   
    (*addr).proto = protos;
    (*addr).sockfd = 0;

    (*addr).sockfd = socket(AF_INET, SOCK_STREAM, (*addr).proto);
    if((*addr).sockfd < 0)
    {
        printf("[-] Socket creation failure...\n\n");
        exit(-1);
    }

    (*addr).sock.sin_family = AF_INET;
    (*addr).sock.sin_port = htons(port);
    
    if(strcmp(ip, "0") == 0 || strcmp(ip, "INADDR_ANY") == 0 || strcmp(ip, "any") == 0 || strcmp(ip, "all") == 0) {
        (*addr).sock.sin_addr.s_addr = INADDR_ANY;
        printf("[i] ANY_IN Set\n");
    }
    else {
        (*addr).sock.sin_addr.s_addr = inet_addr(ip);
        printf("[i] IP Set:\t%s\n", getSocketAddress(addr));
    }
}

/*  Connect to Socket
        Using the AutoSocket Structure passed, the function will connect the socket and do some
        simple error checking
*/
void connectToSocket(struct AutoSocket* addr)
{
    if(socketActive(addr)) 
    {
        int temp = connect((*addr).sockfd, (struct sockaddr*) &((*addr).sock), sizeof((*addr).sock));
        if(temp < 0)
        {
            printf("[-] Connection failure...\n\n");
            exit(-1);
        }
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Bind Socket To Port (Server-Mode ONLY)
        Using the AutoSocket Structure passed, the socket will be bound to the port stored in the socket
*/
void bindSocketToPort(struct AutoSocket* server_addr)
{
    int temp = bind((*server_addr).sockfd, (struct sockaddr*) &((*server_addr).sock), sizeof(struct sockaddr));
    if(temp < 0)
    {
        printf("[-] Failed to bind to port...\n\n");
        exit(-1);
    }
}

/*  Send Message To Socket
        Using the AutoSocket passed, and the message stored in buffer, we can send what ever message is
        input or programed to the device programed in the socket 
*/
int sendMessageToSocket(struct AutoSocket* sock, char* buffer)
{
    if(socketActive(sock)) {
        int temp = send((*sock).sockfd, buffer, strlen(buffer), (*sock).proto);
        if(temp < 0){
            printf("[x] Sending Failed...");
        }
        return temp;
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Receive Message From Socket
        Using the AutoSocket and a variable to store the message in, we can then wait for a message to be sent
        before returning it in the 'buffer' variable. 
        Additionally the function will check incoming buffer for passed commands from client
*/
void receiveMessageFromSocket(struct AutoSocket* sock, char* buffer, size_t maxStr)
{
    if(socketActive(sock))
    {
        bzero(buffer, maxStr);
        recv((*sock).sockfd, buffer, maxStr, (*sock).proto);

        if(strncmp(buffer, "GET", 3) == 0) {
            buffer += 4;

            printf("[+] GET found\n[i] File Requested: %s\n", buffer);
            if(access(buffer, F_OK)){
                buffer[strlen(buffer)-1] = '\0';
                printf("[i] BUFFER: %s<\n", buffer);

                char* name = (char*)malloc(128 * sizeof(char));
                bzero(name, 128); strcat(name, buffer);

                sendFileOverSocket(buffer, sock);

                bzero(buffer, maxStr);
                strcat(buffer, "FILE REQUEST ");
                strcat(buffer, name);
            }
            else {
                sendMessageToSocket(sock, "[x] File Not Found!");
                printf("[-] File Not Found\n");
            }
        }
        else if(strncmp(buffer, "FILE SEND", 9) == 0) {
            printf("[ ] Receiving File...\n");
            receiveFileOverSocket(sock);
        }
        else if(strncmp(buffer, "LIST", 4) == 0) {
            sendFileList(sock);
        }
    }
    else
        printf("[-] Socket Not Open!\n");
}

void sendFileList(struct AutoSocket* sock)
{
    char* buffer = (char*)malloc(128*sizeof(char));
    char* list = (char*)malloc(1024*sizeof(char));
    system("ls > temp.data");
    FILE *fp = fopen("temp.data", "r");
    if(fp!=NULL){
        while(fgets(buffer, 128, fp)){
            if(!(strncmp(buffer, "temp.data", 9) == 0))
                strcat(list, buffer);
        }
        sendMessageToSocket(sock, list);
    }
    else
        printf("[-] Could Not Open File");
    system("rm temp.data");
}

/*  Get Message Time
        Returns the time the following function was called and stores data
        in the 'buffer' passed.

*/
void getMessageTime(char* buffer, size_t maxStr)
{
    time_t timeR;
    struct tm * timeinfo;
    time(&timeR);
    timeinfo = localtime(&timeR);

    bzero(buffer, maxStr);
    strcpy(buffer, asctime(timeinfo));
}

/*  Get Socket Address
        Using the AutoSocket struct passes, the function will return the address of the
        socket in form of a char array.
*/
char* getSocketAddress(struct AutoSocket* addr)
{
    char* out = (char*)malloc(128 * sizeof(char));
    out = inet_ntoa((*addr).sock.sin_addr);
    return out;
}


/*  Listen On Socket
        This will start the listen command on the socket
        Program will then listen to the port programed, returning any errors as they arise
*/
void listenOnSocket(struct AutoSocket* sock)
{
    if(socketActive(sock))
    {
        int temp = listen((*sock).sockfd, (*sock).messageQueue);
        if(temp < 0) {
            printf("[-] Listen Failure\n\n");
            exit(-1);
        }
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Wait for Connection
        Sets the Server-Socket to listen-mode before sleeping the thread.
        Once a connection is received and accepted, then resulting socket data is stored in 'clientSock'
        Basic Error checking implemented.
*/
void waitForConnection(struct AutoSocket* serverSock, struct AutoSocket* clientSock, char* buffer, size_t maxStr)
{
    if(socketActive(serverSock)) 
    {
        socklen_t addrSize = sizeof((*clientSock).sock);
        (*clientSock).sockfd = accept((*serverSock).sockfd, (struct sockaddr*) &((*clientSock).sock), &addrSize);
        if((*clientSock).sockfd < 0) {
            printf("[-] Failure accepting\n\n");
            exit(-1);
        }
        printf("[+] Client Connected!\n");
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Send File Over Socket
        On call, function will send the command "FILE SEND" before sending the filename.
        Then the file is broken down by line and each line is sent in its own message to the receiver.
        Basic Error checking to make sure file is opened.
*/
void sendFileOverSocket(char* fname, struct AutoSocket* sock)
{
    if(socketActive(sock))
    {
        FILE *fp = fopen(fname, "r");
        char *buffer = (char*)malloc(1024 * sizeof(char));
        if(!(fp == NULL)){
            printf("[+] Sending File...\n");
            sendMessageToSocket(sock, "FILE SEND");
            sendMessageToSocket(sock, fname);
            while(fgets(buffer, sizeof(buffer), fp) != NULL)
            {
                sendMessageToSocket(sock, buffer);
            }
            sendMessageToSocket(sock, "-_END-_");
            printf("[ ] File Sent!\n");

            closeSocket(sock);
            printf("[i] Socket Closed\n");
        }
        else {
            printf("[-] File Not Found at Read!\n");
            sendMessageToSocket(sock, "ERROR FILE SEND");
        }
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Receive File Over Socket
        On call, program will prepare to receive a file from the sender.
        First the function will expect to receive a file name.
        Then the file will be sent line by line, where it will then be compiled into the end file.
*/
void receiveFileOverSocket(struct AutoSocket* sock)
{
    if(socketActive(sock))
    {
        char* buffer = (char*)malloc(1024 * sizeof(char)); bzero(buffer, 1024);
        receiveMessageFromSocket(sock, buffer, 1024);

        char* fname = (char*)malloc(128 * sizeof(char)); bzero(fname, 128);
        strcat(fname, buffer);

        printf("[ ] File Being received: %s\n", fname);

        FILE *fp = fopen(fname, "w");
        while(!(strncmp(buffer, "-_END-_", 7) == 0))
        {
            receiveMessageFromSocket(sock, buffer, 1024);
            if(buffer != NULL) {
                fprintf(fp, "%s\n", buffer);
            }
        }
        fclose(fp);
        close((*sock).sockfd);
    }
    else
        printf("[-] Socket Not Open!\n");
}

/*  Socket Active?
        Will return the state of the socket in representation of 'bool'
        TRUE    = active
        FALSE   = not-active
*/
bool socketActive(struct AutoSocket* sock)
{
    bool temp = false;
    int error = 0;
    socklen_t eLen = sizeof(error);
    temp = getsockopt((*sock).sockfd, SOL_SOCKET, SO_ERROR, &error, &eLen) && error;
    return !temp;
}

/*  Close Socket
        Wrapper for close function
*/
void closeSocket(struct AutoSocket* sock)
{
    if(socketActive(sock)) {
        close((*sock).sockfd);
    }
    else
        printf("[-] Socket Not Open!\n");
}

#endif