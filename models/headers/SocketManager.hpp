#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <vector>
#include <string>

//init socket -> prepare the server so it can accept incoming client connections
struct ServerSocketInfo {
    std::string host;
    std::string port;
    std::string server_name;

    ServerSocketInfo(const std::string& h, const std::string& p, const std::string& name)
        : host(h), port(p), server_name(name) {}
};

class SocketManager {
    private:
        std::vector<int> _serverSockets;
    
    public:
        SocketManager();
        ~SocketManager();

        bool initSockets(const std::vector<ServerSocketInfo>& servers);
        const std::vector<int>& getSockets() const;
        void closeSocket();

};

#endif