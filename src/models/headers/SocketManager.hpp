#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//init socket -> prepare the server so it can accept incoming client connections
struct ServerSocketInfo {
    std::string host;
    std::string port;
    std::string serverName;

    ServerSocketInfo(const std::string& h, const std::string& p, const std::string& name)
        : host(h), port(p), serverName(name) {}
};

class SocketManager {
    private:
        std::vector<int> listeningSockets;
        std::map<int, std::string> requestBuffers;
    
    public:
        SocketManager();
        ~SocketManager();

        bool initSockets(const std::vector<ServerSocketInfo>& servers);
        void closeSocket();

        bool isServerSocket(int fd) const;
        const std::vector<int>& getSockets() const;

        void handleClients();
        void handleRequest(int readyServerFd, int epoll_fd);
        void acceptNewClient(int readyServerFd, int epoll_fd);
    

};

#endif