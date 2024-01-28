/*
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                       #
#   Author Info:                                                        #
#       Name: Luke O'Brien                                              #
#       Last Updated: 23 April 2023                                     #
#                                                                       #
#   Program Description:                                                #
#       The following Header file is designed to take traditional       #
#       C Sockets and make them much more intuitive and easier to       #
#       use.                                                            #
#                                                                       #
#       Everything here is a passion project I have been working on     #
#       with no end goal in mind. Suggestions are always welcome        #
#       lobrien@uccs.edu                                                #
#                                                                       #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
*/

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef AUTO_SOCKET_H
#define AUTO_SOCKET_H

struct AutoSocket {
    struct sockaddr_in sock;
    int messageQueue; //optional, for customization
    int sockfd;
    int lineBufferLen; //optional, for customization
    short proto;
};

void createSocket(struct AutoSocket*, char*, const int, short);
void connectToSocket(struct AutoSocket*);
void bindSocketToPort(struct AutoSocket*);
int sendToSocket(struct AutoSocket*, char*);
void receiveFromSocket(struct AutoSocket*, char*);
void checkForCommands(struct AutoSocket*, char*);
void sendFileList(struct AutoSocket*);
void getMessageTime(char*, size_t);
void getSocketAddress(struct AutoSocket*, char*);
void listenOnSocket(struct AutoSocket*);
void waitForConnection(struct AutoSocket*, struct AutoSocket*, char*);
void sendFileOverSocket(char*, struct AutoSocket*);
void receiveFileOverSocket(struct AutoSocket*);
bool socketActive(struct AutoSocket*);
void closeSocket(struct AutoSocket*);

#endif