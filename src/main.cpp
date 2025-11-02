#include "Container.hpp"
#include <iostream>
#include <cstring>
#include "parser.hpp"
#include "utils.hpp"
#include "SocketManager.hpp"

// Forward declaration of helper function from SocketManager.cpp
std::vector<ServerSocketInfo> convertServersToSocketInfo(const std::vector<Server>& servers);

int main(int argc, char **argv) {
    //if no config file found ->> load default built-in confing and print
    //"Warning: No config file provided. Using default configuration."
    if (argc != 2) {
        std::cerr << "Provide a configuration file!" << std::endl;
        return 1;
    }

    try {
        initValidation(argc, argv);
        std::string content = readFile(argv[1]);
        std::vector<Token> tokens = lexer(content);
        checks(tokens);
        Container container = parser(tokens);
        
        std::vector<ServerSocketInfo> socketInfos = convertServersToSocketInfo(container.getServers());

        SocketManager socketManager;
        socketManager.setServers(container.getServers());
        
        // Debug: Print all loaded servers
        // const std::vector<Server>& servers = container.getServers();
        // std::cout << "=== Loaded Servers ===" << std::endl;
        // for (size_t i = 0; i < servers.size(); ++i) {
        //     const Server& server = servers[i];
        //     std::cout << "Server " << i << ":" << std::endl;
            
        //     // Print server names
        //     const std::vector<std::string>& serverNames = server.getServerNames();
        //     std::cout << "  Server names: ";
        //     for (size_t j = 0; j < serverNames.size(); ++j) {
        //         std::cout << serverNames[j];
        //         if (j < serverNames.size() - 1) std::cout << ", ";
        //     }
        //     std::cout << std::endl;
            
        //     // Print listening addresses
        //     const std::vector<ListenCtx>& listens = server.getListens();
        //     std::cout << "  Listens on: ";
        //     for (size_t j = 0; j < listens.size(); ++j) {
        //         std::cout << listens[j].addr << ":" << listens[j].port;
        //         if (j < listens.size() - 1) std::cout << ", ";
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << "======================" << std::endl;
        
        if (!socketManager.initSockets(socketInfos)) {
            std::cerr << "Failed to initialize sockets!" << std::endl;
            return 1;
        }

        std::cout << "Server initialized. Waiting for clients..." << std::endl;
        socketManager.handleClients();

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
