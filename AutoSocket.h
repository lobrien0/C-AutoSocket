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
    int lineBufferLen; //optional, for customization
    short proto;
};

void createSocket(struct AutoSocket*, char*, const int, short);
void connectToSocket(struct AutoSocket*);
void bindSocketToPort(struct AutoSocket*);
int sendMessageToSocket(struct AutoSocket*, char*);
void receiveMessageFromSocket(struct AutoSocket*, char*, size_t);
void checkForCommands(struct AutoSocket*, char*, int);
void sendFileList(struct AutoSocket*);
void getMessageTime(char*, size_t);
void getSocketAddress(struct AutoSocket*, char*, int);
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
void createSocket(struct AutoSocket* addr, char* ip, const int port, short protos)
{   
    // Assigning the set or default values of needed values. 
    (*addr).proto = protos;
    (*addr).lineBufferLen = 1024; //can be changed manually
    (*addr).messageQueue = 3; //can be changed manually
    (*addr).sockfd = 0;

    // Creates the socket and sets File Descripter in struct
    (*addr).sockfd = socket(AF_INET, SOCK_STREAM, (*addr).proto);
    if((*addr).sockfd < 0)
    {
        printf("[-] Socket creation failure...\n\n");
        exit(-1);
    }

    // Declaring Address-Form and setting port
    (*addr).sock.sin_family = AF_INET;
    (*addr).sock.sin_port = htons(port);
    
    // Determins if IP is listening for 'any' IP or needs to be set to a specific address
    if(strcmp(ip, "0") == 0 || strcmp(ip, "INADDR_ANY") == 0 || strcmp(ip, "any") == 0 || strcmp(ip, "all") == 0) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
        printf("[i] ANY_IN Set\n");
    }
    // Sets specific address
    else {
        (*addr).sock.sin_addr.s_addr = inet_addr(ip);
        printf("[i] IP Set\n");
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
}

/*  Bind Socket To Port (Server ONLY)
        Using the AutoSocket Structure passed, the socket will be bound to the port stored in the socket
*/
void bindSocketToPort(struct AutoSocket* server_addr)
{
    if(socketActive(server_addr)){
        int temp = bind((*server_addr).sockfd, (struct sockaddr*) &((*server_addr).sock), sizeof(struct sockaddr));
        if(temp < 0)
        {
            printf("[-] Failed to bind to port...\n\n");
            exit(-1);
        }
    }
}

/*  Send Message To Socket
        Using the AutoSocket passed, and the message stored in buffer, we can send what ever message is
        input or programed to the device programed in the socket 
*/
int sendMessageToSocket(struct AutoSocket* sock, char* buffer)
{
    int temp;
    if(socketActive(sock)) {
        temp = send((*sock).sockfd, buffer, strlen(buffer), (*sock).proto);
        if(temp < 0){
            printf("[x] Sending Failed...");
        }
        sleep(0.001);
    }
    return temp;
}

/*  Receive Message From Socket
        Using the AutoSocket and a variable to store the message in, we can then wait for a message to be sent
        before returning it in the 'buffer' variable. 
        Additionally the function will reference 'checkForCommands' (see function comments for details)
*/
void receiveMessageFromSocket(struct AutoSocket* sock, char* buffer, size_t maxStr)
{
    if(socketActive(sock))
    {
        bzero(buffer, maxStr);
        int temp = recv((*sock).sockfd, buffer, maxStr, (*sock).proto);
        if(temp < 0){
            printf("[-] Receive Failed...\n");
        }
        else {
            checkForCommands(sock, buffer, maxStr);
        }
    }
}

/*  Check For Commands
        Taking input in the form of a character pointer buffer, the function then checks the first few
        character slots for pre-specified commands that can be issued from a remote client application

    Commands:
        GET {file}       - Issued by the user/client; Will request a specified file from server
        LIST             - Issued by the user/Client; Will request the server to list available files
        FILE SEND {file} - Issued by the server; Tells client to prepare to receive a file
*/
void checkForCommands(struct AutoSocket* sock, char* buffer, int maxStr)
{
    // If Client is Requesting file using "GET" Command
    if(strncmp(buffer, "GET", 3) == 0) {
            buffer += 4; //Increments string pointer past the "GET "
            printf("[+] GET found\n[i] File Requested: %s\n", buffer);
            
            //If the File exists
            if(access(buffer, F_OK)){
                buffer[strlen(buffer)-1] = '\0'; // Removes endline
                printf("[i] BUFFER: %s<\n", buffer); // Prints where end of file name is

                // space for file name allocated for later buffer use.
                char* name = (char*)malloc(128 * sizeof(char));
                bzero(name, 128); strcat(name, buffer);

                sendFileOverSocket(buffer, sock);

                bzero(buffer, maxStr);
                strcat(buffer, "FILE REQUEST ");
                strcat(buffer, name);
                free(name);
            }
            // If file couldn't be found:
            else {
                sendMessageToSocket(sock, "[x] File Not Found!");
                printf("[-] File Not Found\n");
            }
        }
        // If Command is "FILE SEND" Prepare to read in a file form socket
        else if(strncmp(buffer, "FILE SEND", 9) == 0) {
            printf("[ ] Receiving File...\n");
            receiveFileOverSocket(sock);
        }
        // If Command is "LIST" then send a list of files
        else if(strncmp(buffer, "LIST", 4) == 0) {
            sendFileList(sock);
        }
}

/*  Send File List
        On call, the function will compile a list of available files before sending
        that list to the remote client.
*/
void sendFileList(struct AutoSocket* sock)
{
    // Allocating data for storage
    char* buffer = (char*)malloc(128*sizeof(char));
    char* list = (char*)malloc(1024*sizeof(char));

    system("ls > temp.data"); // Using linux comand and exporting output to temp file
    FILE *fp = fopen("temp.data", "r"); // Open Temp File
    
    //Reads file line by line into 'buffer', which then appends to 'list'
    if(fp!=NULL){
        while(fgets(buffer, 128, fp)){
            if(!(strncmp(buffer, "temp.data", 9) == 0))
                strcat(list, buffer);
        }
        sendMessageToSocket(sock, list); // Sends List to client
    }
    // Error with Temp File
    else
        printf("[-] Could Not Open File");
    system("rm temp.data");
    free(buffer);
    free(list);
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
    strcat(buffer, asctime(timeinfo));
}

/*  Get Socket Address
        Using the AutoSocket struct passes, the function will return the address of the
        socket in form of a char array.
*/
void getSocketAddress(struct AutoSocket* addr, char* buffer, int maxStr)
{
    bzero(buffer, maxStr);
    strcat(buffer, inet_ntoa((*addr).sock.sin_addr));
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
        char *buffer = (char*)malloc((*sock).lineBufferLen * sizeof(char)); // Buffer User defined or Default[1024]

        // If files exists
        if(!(fp == NULL))
        {
            printf("[+] Sending File...\n");
            sendMessageToSocket(sock, "FILE SEND"); // Sends command to let client know file is about to send
            sendMessageToSocket(sock, fname); // Sends file name to client

            // While there is still another line to read in file, send it to client
            while(fgets(buffer, (*sock).lineBufferLen, fp) != NULL)
            {
                sendMessageToSocket(sock, buffer);
            }
            sendMessageToSocket(sock, "-_END-_"); // A message to signal end of file
            printf("[ ] File Sent!\n");

            closeSocket(sock); // Can also be signaled to 
            printf("[i] Socket Closed\n");
        }

        // If Error opening file
        else
        {
            printf("[-] File Not Found at Read!\n");
            sendMessageToSocket(sock, "ERROR FILE SEND");
        }

        free(buffer);
    }
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
        // Buffer allocation size determined by user or by default[1024]
        char* buffer = (char*)malloc((*sock).lineBufferLen * sizeof(char)); bzero(buffer, (*sock).lineBufferLen);
        receiveMessageFromSocket(sock, buffer, (*sock).lineBufferLen); //Get Name

        printf("[ ] File Being received: %s\n", buffer);

        FILE *fp = fopen(buffer, "w");
        while(!(strncmp(buffer, "-_END-_", 7) == 0) || socketActive(sock))
        {
            receiveMessageFromSocket(sock, buffer, 1024);
            if(buffer != NULL) {
                fprintf(fp, "%s\n", buffer);
            }
        }
        fclose(fp);
        closeSocket(sock);
        free(buffer);
    }
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
    if(temp)
        printf("[-] Socket not open...\n");
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
}

#endif