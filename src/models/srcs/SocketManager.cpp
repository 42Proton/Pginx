#include "SocketManager.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

SocketManager::SocketManager() {}

SocketManager::~SocketManager() {
    closeSocket();
}

// Helper function to convert Server objects to ServerSocketInfo
std::vector<ServerSocketInfo> convertServersToSocketInfo(const std::vector<Server>& servers) {
    std::vector<ServerSocketInfo> socketInfos;
    
    for (size_t i = 0; i < servers.size(); ++i) {
        const Server& server = servers[i];
        const std::vector<ListenCtx>& listens = server.getListens();
        const std::vector<std::string>& serverNames = server.getServerNames();
        
        // Get server name (use first one or default)
         std::string serverName = "";
        if (!serverNames.empty()) {
            serverName = serverNames[0];
        }
        
        // Create ServerSocketInfo for each listen directive
        for (size_t j = 0; j < listens.size(); ++j) {
            const ListenCtx& listen = listens[j];

            ServerSocketInfo info(
                listen.addr,                    // host
                intToString(listen.port),       // port as string
                serverName                      // server name
            );
            
            socketInfos.push_back(info);
        }
    }
    
    return socketInfos;
}

bool SocketManager::initSockets(const std::vector<ServerSocketInfo>& servers) {
    // Clean up any existing sockets first
    // closeSocket();
    
    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerSocketInfo &server = servers[i];

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        std::string port_str = server.port;

        if (getaddrinfo(server.host.empty() ? NULL : server.host.c_str(),
            port_str.c_str(), &hints, &res) != 0) {
                std::cerr << "getaddrinfo failed for server " << i << "\n";
                // Clean up any sockets created so far
                closeSocket();
                return false;
        }

        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            std::cerr << "socket creation failed for server " << i << "\n";
            freeaddrinfo(res);
            // Clean up any sockets created so far
            closeSocket();
            return false;
        }

        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "setsockopt failed for server " << i << "\n";
            close(sockfd);
            freeaddrinfo(res);
            // Clean up any sockets created so far
            closeSocket();
            return false;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            std::cerr << "bind failed for server " << i << "\n";
            close(sockfd);
            freeaddrinfo(res);
            // Clean up any sockets created so far
            closeSocket();
            return false;
        }

        if (listen(sockfd, 10) == -1) {
            std::cerr << "listen failed for server " << i << "\n";
            close(sockfd);
            freeaddrinfo(res);
            // Clean up any sockets created so far
            closeSocket();
            return false;
        }

        freeaddrinfo(res);
        _serverSockets.push_back(sockfd);

        std::cout << "Server " << i << " listening on port " << server.port << " (fd=" << sockfd << ")\n";
    }
    return true;
}

const std::vector<int>& SocketManager::getSockets() const {
    return _serverSockets;
}

void SocketManager::closeSocket() {
    for (size_t i = 0; i < _serverSockets.size(); ++i) {
        if (_serverSockets[i] != -1) {
            close(_serverSockets[i]);
        }
    }
    _serverSockets.clear();
}