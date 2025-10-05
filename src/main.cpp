#include "Container.hpp"
#include <iostream>
#include <cstring>
#include "parser.hpp"
#include "utils.hpp"
#include "SocketManager.hpp"

// Forward declaration of helper function from SocketManager.cpp
std::vector<ServerSocketInfo> convertServersToSocketInfo(const std::vector<Server>& servers);

void printContainer(const Container &container)
{
    std::cout << "\n=== PARSED CONFIGURATION ===" << std::endl;
    std::cout << "Number of servers: " << container.getServers().size() << std::endl;

    for (size_t i = 0; i < container.getServers().size(); ++i)
    {
        const Server &server = container.getServers()[i];
        std::cout << "\n--- Server " << (i + 1) << " ---" << std::endl;

        // Print listen addresses
        const std::vector<ListenCtx> &listens = server.getListens();
        std::cout << "Listen addresses (" << listens.size() << "):" << std::endl;
        for (size_t j = 0; j < listens.size(); ++j)
        {
            std::cout << "  " << listens[j].addr << ":" << listens[j].port << std::endl;
        }

        // Print server names
        const std::vector<std::string> &serverNames = server.getServerNames();
        std::cout << "Server names (" << serverNames.size() << "):" << std::endl;
        for (size_t j = 0; j < serverNames.size(); ++j)
        {
            if (!serverNames[j].empty())
                std::cout << "  " << serverNames[j] << std::endl;
        }

        // Print root
        std::cout << "Root: " << server.getRoot() << std::endl;

        // Print client max body size
        std::cout << "Client max body size: " << server.getClientMaxBodySize() << " bytes" << std::endl;

        // Print auto index
        std::cout << "Auto index: " << (server.getAutoIndex() ? "on" : "off") << std::endl;

        // Print locations
        const std::vector<LocationConfig> &locations = server.getLocations();
        std::cout << "Locations (" << locations.size() << "):" << std::endl;
        for (size_t j = 0; j < locations.size(); ++j)
        {
            const LocationConfig &location = locations[j];
            std::cout << "  Location: " << location.getPath() << std::endl;
            std::cout << "    Root: " << location.getRoot() << std::endl;
            std::cout << "    Auto index: " << (location.getAutoIndex() ? "on" : "off") << std::endl;

            const std::vector<std::string> &methods = location.getMethods();
            std::cout << "    Allowed methods (" << methods.size() << "): ";
            for (size_t k = 0; k < methods.size(); ++k)
            {
                std::cout << methods[k];
                if (k < methods.size() - 1)
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "=== END CONFIGURATION ===" << std::endl;
}

// int main(int argc, char **argv)
// {
//     // TODO: handling default values
//     if (argc == 1)
//         return 0;
//     if (argc != 2)
//     {
//         std::cerr << "Provide a Configuration file! " << std::endl;
//         return 1;
//     }
//     try
//     {
//         initValidation(argc, argv);
//         std::string content = readFile(argv[1]);
//         std::vector<Token> tokens = lexer(content);
//         checks(tokens);

//         std::cout << "Parsing configuration..." << std::endl;
//         Container container = parser(tokens);

//         // Print the parsed container contents
//         // printContainer(container);
        
//         // Initialize socket manager
//         SocketManager socketManager;
        
//         std::vector<ServerSocketInfo> socketInfos = convertServersToSocketInfo(container.getServers());
//         for (size_t i = 0; i < socketInfos.size(); i++) {
//             const ServerSocketInfo& info = socketInfos[i];

//             std::cout << "ServerSocketInfo " << i << ":\n";
//             std::cout << "  host       : " << info.host << "\n";
//             std::cout << "  port       : " << info.port << "\n";
//             std::cout << "  serverName : " << info.serverName << "\n";
//             std::cout << "---------------------------\n";
//         }
        
//         // Initialize sockets
//         std::cout << "Initializing sockets..." << std::endl;
//         if (socketManager.initSockets(socketInfos)) {
//             std::cout << "All sockets initialized successfully!" << std::endl;
            
//             // Get the socket file descriptors for further use
//             const std::vector<int>& sockets = socketManager.getSockets();
//             std::cout << "Created " << sockets.size() << " server sockets" << std::endl;
            
//             // Test accept function - use first socket for testing
//             if (!sockets.empty()) {
//                 int server_socket = sockets[0];
//                 std::cout << "Server ready! Waiting for connections on socket " << server_socket << "..." << std::endl;
                
//                 // Simple loop to accept ONE connection for testing
//                 std::string clientInfo;
//                 int client_fd = socketManager.acceptConnection(server_socket, clientInfo);
                
//                 if (client_fd != -1) {
//                     std::cout << "Connection accepted! Client: " << clientInfo << std::endl;
                    
//                     // Simple echo test: read data and send it back
//                     char buffer[1024];
//                     ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
//                     if (bytes_read > 0) {
//                         buffer[bytes_read] = '\0';
//                         std::cout << "Received " << bytes_read << " bytes: " << buffer << std::endl;
                        
//                         // Echo the data back to client
//                         const char* response = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
//                         send(client_fd, response, strlen(response), 0);
//                         std::cout << "Sent HTTP response back to client" << std::endl;

//                     } 
//                     else if (bytes_read == 0)
//                         std::cout << "Client disconnected immediately" << std::endl; 
//                     else
//                         perror("read failed");
                    
//                     close(client_fd);
//                     std::cout << "Client connection closed" << std::endl;
//                 } 
//                 else
//                     std::cerr << "Failed to accept connection!" << std::endl;
//             }
            
//         } else {
//             std::cerr << "Failed to initialize sockets!" << std::endl;
//             return 1;
//         }

//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Provide a configuration file!" << std::endl;
        return 1;
    }

    try {
        // 1 Parse configuration file
        initValidation(argc, argv);
        std::string content = readFile(argv[1]);
        std::vector<Token> tokens = lexer(content);
        checks(tokens);
        Container container = parser(tokens);

        // 2 Convert parsed servers to socket info
        std::vector<ServerSocketInfo> socketInfos = convertServersToSocketInfo(container.getServers());

        // 3 Initialize sockets
        SocketManager socketManager;
        if (!socketManager.initSockets(socketInfos)) {
            std::cerr << "Failed to initialize sockets!" << std::endl;
            return 1;
        }

        std::cout << "Server initialized. Waiting for clients..." << std::endl;
        // 4  Enter main server loop to handle multiple clients
        //this function is the main loop that :
            // - Uses poll() or select()
            // - Accepts new clients
            // - Receives requests via recv()
            // - Calls parseHttpRequest() for each client
            // - Sends the HTTP response back
        //parseHttpRequest(buffer) is a separate function that:
            // Converts the raw recv() buffer into a structured object (HttpRequest)
        // MultipleClients(socketManager);
        socketManager.handleClients();

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
