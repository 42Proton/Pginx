#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>

#define MAX_HEADER_SIZE 4096                               // 4 KB
#define MAX_BODY_SIZE 65536                                // 64 KB
#define MAX_REQUEST_SIZE (MAX_HEADER_SIZE + MAX_BODY_SIZE) // 68 KB

// init socket -> prepare the server so it can accept incoming client connections
struct ServerSocketInfo
{
    std::string host;
    std::string port;
    std::string serverName;

    ServerSocketInfo(const std::string &h, const std::string &p, const std::string &name)
        : host(h), port(p), serverName(name)
    {
    }
};

class SocketManager
{
  private:
    std::vector<int> listeningSockets;
    std::map<int, std::string> requestBuffers;
    std::map<int, time_t> lastActivity;
    std::map<int, std::string> sendBuffers;
    static const int CLIENT_TIMEOUT = 60;

  public:
    SocketManager();
    ~SocketManager();

    bool initSockets(const std::vector<ServerSocketInfo> &servers);
    void closeSocket();

    bool isServerSocket(int fd) const;
    const std::vector<int> &getSockets() const;

    void handleClients();
    void handleRequest(int readyServerFd, int epoll_fd);
    void acceptNewClient(int readyServerFd, int epoll_fd);
    void handleTimeouts(int epoll_fd);
    void sendBuffer(int fd, int epfd);
    bool isRequestTooLarge(int fd);
    bool isHeaderTooLarge(int fd);
    bool isRequestLineMalformed(int fd);
    bool hasNonPrintableCharacters(int fd);
    void sendHttpError(int fd, const std::string &status, int epfd);
    bool isBodyTooLarge(int fd);
};

#endif