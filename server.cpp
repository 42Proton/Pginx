#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

//server

int main() {
    std::cout << "starting the server ..." << std::endl;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0); //"Create an IPv4 socket, that is stream-oriented (TCP), using the default protocol (TCP)."
    if (server_socket == -1) {
        std::cerr << "Error creating the socket !" << std::endl;
        return 1;
    }

    // struct used for IPv4 sokcet addresses
    //It stores the IP address + port + family (protocol type) for the socket.
   
   
    sockaddr_in serverAddr;
    //sin_family -> specifies the address family.
    //AF_INET -> IPv4
    serverAddr.sin_family = AF_INET;
    //INADDR_ANY -> bind to all available network interfaces on this machine;
    // holds the IP address, sin_addr is a struct
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // sin_port -> holds the port number
    //4000 is the port where the server will listen.
    //htons -> It converts the number to network byte order (big-endian),
    serverAddr.sin_port = htons(4000);
   
    //int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    //It wants a pointer to struct sockaddr, which is a generic socket address type.
    //but sockaddr_in is IPv4-specific, not the generic sockaddr.
    //so we need to cast -> (struct sockaddr*)&serverAddr
    if (bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding the socket !" << std::endl;
        close(server_socket);
        return 1;
    }

    //int listen(int socket, int backlog);
    //listen for incoming connections
    //Before listen(), your socket is just bound to an IP/Port but isn’t ready to accept connections
    //Each client that wants to connect is put into a queue inside the OS,
    //waiting for your program to call accept(). -> backlog sets the maximum length of that queue.
    //how listen works?
    //- The kernel creates a queue in memory associated with this socket.
    //- This queue stores connection requests from clients that try to connect but haven’t been accepted yet.
    //- The length of this queue is limited by the backlog parameter.
    listen(server_socket, 1);

    //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    //accept-> Returns a new socket file descriptor (not the same as server_socket).
    int client_socket = accept(server_socket, NULL, NULL);

    //The buffer will temporarily store data received from the client.
    char buff[1024];
    while (true) {
        //init the buff
        memset(buff, 0, sizeof(buff));

        //client_socket → the connected socket (returned by accept).
        // buff → pointer to memory where received data will be stored.
        // 0 → flags (we pass 0 for default behavior).
        recv(client_socket, buff, sizeof(buff), 0);  // read data from client
        std::cout << "Message from client: " << buff;
        if (strcmp(buff, "exit\n") == 0)
            break ;
    }
    close(server_socket);
    return 0;
}

//how the data actually travels from the client buffer to the server buffer?
//1️⃣ Client side: send(client_socket, &mess, strlen(mess), 0);
//mess is in user-space memory on the client machine.
//When you call send(), the kernel copies the data from mess into the kernel’s TCP send buffer.
//Then the TCP stack takes over:
//  - Then the TCP stack takes over:
//      f- Splits the data into packets.

