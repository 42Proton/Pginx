#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    std::cout << "Starting the Client ..." << std::endl;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating the socket ..." << std::endl;
        return 1;
    }
    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;

    //the client knows the server is listening on port 4000.
    //The client “knows” the server because you
    //explicitly tell it the server’s IP and port in the sockaddr_in struct
    clientAddr.sin_port = htons(4000);
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    
    //Client sends a SYN packet to the server IP:port.
    //Server receives it on the listening socket (bind() + listen()).
    //Server queues it.
    //Server calls accept() → creates a new socket for that client.
    //TCP handshake completes → connection established.
    connect(client_socket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

    char mess[1024];
    while (true) {
        std::cout << "Enter the message to send : ";

        //reads a line of text from the user (keyboard).
        //Stores it into mess.
        //Stops at newline (\n) or when the buffer is full.
        fgets(mess, sizeof(mess), stdin);

        //Sends the message over the network to the server.
        //client_socket → the socket connected to the server.
        //&mess → pointer to the message buffer
        //Copies the bytes from mess to the kernel buffe
        //TCP ensures the server receives them in order.
        send(client_socket, &mess, strlen(mess), 0);
        if (strcmp(mess, "exit\n") == 0)
            break ;
        
        //Clear buffer
        memset(mess, 0, sizeof(mess));
    }
    close(client_socket);
    return 0;
}
