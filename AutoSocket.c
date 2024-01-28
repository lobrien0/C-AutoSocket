#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <time.h>
#include <dirent.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "AutoSocket.h"

/*  Create Socket
        Using the structure AutoSock the program will first set all the parameters into their 
        proper structures before creating and storing the resulting entity.
        We set the IP, Port, Protocol, and Length of Backup Queue (only used for server-mode)
*/
void createSocket(struct AutoSocket* sock, char* ip, const int port, short protos)
{   
    // Assigning the set or default values of needed values. 
    (*sock).proto = protos;
    (*sock).lineBufferLen = 2048; //can be changed manually
    (*sock).messageQueue = 3; //can be changed manually
    (*sock).sockfd = 0;

    // Creates the socket and sets File Descripter in struct
    (*sock).sockfd = socket(AF_INET, SOCK_STREAM, (*sock).proto);
    if((*sock).sockfd < 0)
    {
        perror("[x] Socket creation failure...");
        exit(-1);
    }

    // Declaring Address-Form and setting port
    (*sock).sock.sin_family = AF_INET;
    (*sock).sock.sin_port = htons(port);
    
    // Determins if IP is listening for 'any' IP or needs to be set to a specific address
    if(strcmp(ip, "0") == 0 || strcmp(ip, "INADDR_ANY") == 0 || strcmp(ip, "any") == 0 || strcmp(ip, "all") == 0) {
        (*sock).sock.sin_addr.s_addr = INADDR_ANY;
        printf("[i] ANY_IN Set\n");
    }
    // Sets specific address
    else {
        (*sock).sock.sin_addr.s_addr = inet_addr(ip);
        printf("[i] IP Set\n");
    }
}

/*  Connect to Socket
        Using the AutoSocket Structure passed, the function will connect the socket and do some
        simple error checking
*/
void connectToSocket(struct AutoSocket* sock)
{
    if(socketActive(sock)) 
    {
        int temp = connect((*sock).sockfd, (struct sockaddr*) &((*sock).sock), sizeof((*sock).sock));
        if(temp < 0)
        {
            perror("[x] Connection failure...");
        }
    }
}

/*  Bind Socket To Port ("Server" ONLY)
        Using the AutoSocket Structure passed, the socket will be bound to the port stored in the socket
*/
void bindSocketToPort(struct AutoSocket* sock)
{
    if(socketActive(sock)){
        int temp = bind((*sock).sockfd, (struct sockaddr*) &((*sock).sock), sizeof(struct sockaddr));
        if(temp < 0)
        {
            perror("[x] Failed to bind to port...");
            exit(-1);
        }
    }
}

/*  Send Message To Socket
        Using the AutoSocket passed, and the message stored in buffer, we can send what ever message is
        input or programed to the device programed in the socket 
*/
int sendToSocket(struct AutoSocket* sock, char* buffer)
{
    int temp;
    if(socketActive(sock)) {
        temp = send((*sock).sockfd, buffer, strlen(buffer), (*sock).proto);
        if(temp < 0){
            perror("[x] Sending Failed...");
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
void receiveFromSocket(struct AutoSocket* sock, char* buffer)
{
    if(socketActive(sock))
    {
        bzero(buffer, (*sock).lineBufferLen);
        int temp = recv((*sock).sockfd, buffer, (*sock).lineBufferLen, 0);
        if(temp < 0){
            perror("[x] Receive Failed...");
        }
        else if(temp == 0){
            closeSocket(sock);
        }
        else {
            checkForCommands(sock, buffer);
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
void checkForCommands(struct AutoSocket* sock, char* buffer)
{
    // If Client is Requesting file using "GET" Command
    if(strncmp(buffer, "GET", 3) == 0) 
    {
        buffer += 4; //Increments string pointer past the "GET "
        printf("[+] GET found\n[i] File Requested: %s\n", buffer);
        
        //If the File exists
        if(access(buffer, F_OK)){
            buffer[strlen(buffer)-1] = '\0'; // Removes endline
            printf("[i] BUFFER: %s<\n", buffer); // Prints where end of file name is

            // Space for name allocated, and buffer copied over
            char name[255];
            bzero(name, 255);
            strcat(name, buffer);

            sendFileOverSocket(buffer, sock);

            // Clear the buffer, and input "FILE REQUEST <filename>"
            bzero(buffer, (*sock).lineBufferLen);
            strcat(buffer, "FILE REQUEST ");
            strcat(buffer, name);
        }
        // If file couldn't be found:
        else {
            sendToSocket(sock, "[x] File Not Found!");
            perror("[x] File Not Found\n");
        }
    }
    // If Command is "FILE SEND" Prepare to read in a file form socket
    else if(strncmp(buffer, "FILE SEND", 9) == 0) {
        receiveFileOverSocket(sock);
    }
    
    // If Command is "LIST" then send a list of files
    else if(strncmp(buffer, "LIST", 4) == 0) 
        sendFileList(sock);

    // If Command is "EOL" then server will shutdown
    else if(strncmp(buffer, "EOL", 3) == 0) {
        printf("[-] Remote Shutdown Initiated\n");
        sendToSocket(sock, "[-] Server Shutdown Initiated");
        closeSocket(sock);
        exit(1);
    }
}

/*  Send File List
        On call, the function will compile a list of available files before sending
        that list to the remote client.
*/
void sendFileList(struct AutoSocket* sock)
{
   // Creates directory struct and open the current directory
   struct dirent *dir;
   DIR *d = opendir(".");

   char buffer[2048];
   bzero(buffer, 2048);
   strcat(buffer, "Files:\n----- ----- -----\n");

   // If directory is not NULL
   if (d) {
      // While there another file, read the next file
      while ((dir = readdir(d)) != NULL) {
         // If NOT a directory
         if((*dir).d_type != DT_DIR)
         {
            strcat(buffer, (*dir).d_name); // Add file to buffer
            strcat(buffer, "\n"); // Move to new line
         }
      }
      closedir(d); // close the directory
      buffer[strlen(buffer)-1] = '\0'; // Remove last '\n'
      sendToSocket(sock, buffer);
   }
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
void getSocketAddress(struct AutoSocket* sock, char* buffer)
{
    bzero(buffer, (*sock).lineBufferLen);
    strcat(buffer, inet_ntoa((*sock).sock.sin_addr));
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
            perror("[x] Listen Failure\n\n");
            exit(-1);
        }
    }
}

/*  Wait for Connection
        Sets the Server-Socket to listen-mode before sleeping the thread.
        Once a connection is received and accepted, then resulting socket data is stored in 'clientSock'
        Basic Error checking implemented.
*/
void waitForConnection(struct AutoSocket* serverSock, struct AutoSocket* clientSock, char* buffer)
{
    if(socketActive(serverSock)) 
    {
        socklen_t addrSize = sizeof((*clientSock).sock);
        (*clientSock).sockfd = accept((*serverSock).sockfd, (struct sockaddr*) &((*clientSock).sock), &addrSize);
        if((*clientSock).sockfd < 0) {
            perror("[x] Failure accepting\n\n");
        }
        else
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
        // If no custom length for lineBufferLength is set, set to 2048
        if((*sock).lineBufferLen == 0)
            (*sock).lineBufferLen = 2048;
        
        //Open specified file for reading
        FILE *fp = fopen(fname, "rb");
        long fsize = ftell(fp);

        char *buffer = (char*)malloc((*sock).lineBufferLen * sizeof(char)); // Buffer User defined or Default[2048]
        bzero(buffer, (*sock).lineBufferLen);

        // If files exists
        if(fp != NULL)
        {
            printf("[+] Sending File...\n");
            sendToSocket(sock, "FILE SEND"); // Sends command to let client know file is about to send
            sendToSocket(sock, fname); // Sends file name to client

            // Program breaks file down by Byte and reads while the end of file isn't hit
            while(!feof(fp))
            {
                fread(buffer, (*sock).lineBufferLen, 1, fp);
                buffer[(*sock).lineBufferLen] = '\0';
                sendToSocket(sock, buffer);
                bzero(buffer, (*sock).lineBufferLen);
            }
            printf("[ ] File Sent!\n");
        }

        // If Error opening file
        else
        {
            perror("[x] File Not Found at Read!\n");
            sendToSocket(sock, "ERROR FILE SEND");
        }

        free(buffer);
        fclose(fp);
        closeSocket(sock);
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
        // Buffer allocation size determined by user or by default[2048]
        char buffer[(*sock).lineBufferLen];
        bzero(buffer, (*sock).lineBufferLen);
        receiveFromSocket(sock, buffer); //Get Name

        FILE *fp = fopen(buffer, "wb");
        bzero(buffer, (*sock).lineBufferLen);

        // While the EOF flag has not been sent
        while(socketActive(sock))
        {
            receiveFromSocket(sock, buffer);

            int x, endIndex = 0; 
            for(x = (*sock).lineBufferLen-1; x >= 0; x--) {
                if(buffer[x] != '\0') {
                    endIndex = x+1;
                    break;
                }
            }

            fwrite(buffer, endIndex, 1, fp);
            bzero(buffer, (*sock).lineBufferLen);
        }
        closeSocket(sock);
        fclose(fp);
    }
}

/*  Socket Active?
        Will return the state of the socket in representation of 'bool'
        TRUE    = active
        FALSE   = not-active
*/
bool socketActive(struct AutoSocket* sock)
{
    int err;
    socklen_t errlen = sizeof(err);

    if(getsockopt((*sock).sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0)
        return false;
    if(err == 0)
        return true;
    return false;
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