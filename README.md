# AutoSocket

AutoSocket is a header-file currently in the works.  
The primary goal of this header-file is to improve ease of use with sockets from the standard C Libraries

## Current Progress

Currently the functionality is limited and connection issues aren't unheard of.  
But the project only has a few hours or work time put into it, so with time I expect to see the overall usability of this socket to be much greater.

### Error checking

There is some error checking implemented into the AutoSockets, but it is fairly limited and will be improved as time goes on

## How to Implement

A quick example of how to start using a socket can be seen in both `Server.c` and `Client.c`

Long story short, you have to create a _struct_ of the AutoSocket and call on functions to manipulate and use the sockets

Ex:
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

## Questions?

If you have any questions about my intentions or plans for this, please reach out to me at the following  
[lobrien@uccs.edu]()