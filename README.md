# AutoSocket

AutoSocket is a header-file that aims to improve ease-of-use of the C library sockets. It does this by pre-defining a set of actions, adding error checking and utility functions and few other tricks to make the process much easier.

## Current Progress

The main functionality of the program is tested and functioning. File transfer, messages, and commands all work. 

Unfortunately there is an issue where over WANs the connection struggles. I am actively working on resolving this issue

# How to Implement

A quick example of how to start using a socket can be seen in both `Server.c` and `Client.c`

Long story short, you have to create a _struct_ of the AutoSocket and call on functions to manipulate and use the sockets

# Example:
```C
#include <stdio.h>
#include <string.h>
#include "AutoSocket.h"

int main()
{
    struct AutoSocket sock;
    char message[] = "Hello World";

    // Socket, Ip, Port, Protocol, socket queue
    createSocket(&sock, "192.168.1.1", 8000, 0, 0);
    connectToSocket(&sock);
    sendMessageToSocket(&sock, message);
}
```

# Code Options:

## Struct options

#### __lineBufferLength__

This variable of _type_ `int` is used as the maximum buffer length.  
Default value is: `1024`

If your files or messages will be longer than the default value you will need to manually change the value to something larger.  
_Or smaller if you don't need the full size of the default_

If you want to change your `lineBufferLength` see this example

```c
int main()
{
    struct AutoSocket mySock;

    char ip[] = "127.0.0.1";
    int protocol = 0;
    int port = 8000;
    
    // In order to change the value of 'lineBufferLength'
    // you must do so AFTER 'createSocket' has been run
    createSocket(&mySock, ip, port, protocol);

    mySock.lineBufferLength = 2048;
    // If you are not calling 'createSocket' you may set
    // the value anytime after the AutoSocket instance was made.
}
```

#### __messageQueue__

This variable of _type_ `int` is used to define maximum number of clients waiting to connect to the _Listening_ entity.  
Default Value is: `0`

If you don't plan on using it, you can leave the default, otherwise you will need to manually set the value anytime after you've created the `AutoSocket` _struct_

```c
int main()
{
    // can be set anytime after the struct is made
    struct AutoSocket mySock;
    mySock.messageQueue = 10;

    char ip[] = "127.0.0.1";
    int protocol = 0;
    int port = 8000;

    createSocket(mySock, ip, port, protocol);
}
```

## Questions?

If you have any questions about my intentions or plans for this, please reach out to me at the following  
[lobrien@uccs.edu]()