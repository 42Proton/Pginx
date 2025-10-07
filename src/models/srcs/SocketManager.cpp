#include "SocketManager.hpp"
#include "Server.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>

SocketManager::SocketManager() {}

SocketManager::~SocketManager() {
    closeSocket();
}

std::string initToString(int n) {
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

std::vector<ServerSocketInfo> convertServersToSocketInfo(const std::vector<Server>& servers) {
    std::vector<ServerSocketInfo> socketInfos;
    
    for (size_t i = 0; i < servers.size(); ++i) {
        const Server& server = servers[i];
        const std::vector<ListenCtx>& listens = server.getListens();
        const std::vector<std::string>& serverNames = server.getServerNames();
        
        std::string serverName = "";
        if (!serverNames.empty()) {
            serverName = serverNames[0];
        }
        for (size_t j = 0; j < listens.size(); ++j) {
            const ListenCtx& listen = listens[j];

            ServerSocketInfo info(
                listen.addr,
                initToString(listen.port),
                serverName
            );
            
            socketInfos.push_back(info);
        }
    }
    
    return socketInfos;
}

bool SocketManager::initSockets(const std::vector<ServerSocketInfo>& servers) {

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
                closeSocket();
                return false;
        }

        int listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listen_fd == -1) {
            std::cerr << "socket creation failed for server " << i << "\n";
            freeaddrinfo(res);
            closeSocket();
            return false;
        }

        int opt = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            std::cerr << "setsockopt failed for server " << i << "\n";
            close(listen_fd);
            freeaddrinfo(res);
            closeSocket();
            return false;
        }

        if (bind(listen_fd, res->ai_addr, res->ai_addrlen) == -1) {
            std::cerr << "bind failed for server " << i << ": " << strerror(errno) << "\n";
            std::cerr << "Trying to bind to " << server.host << ":" << server.port << "\n";
            close(listen_fd);
            freeaddrinfo(res);
            closeSocket();
            return false;
        }

        if (listen(listen_fd, 10) == -1) {
            std::cerr << "listen failed for server " << i << "\n";
            close(listen_fd);
            freeaddrinfo(res);
            closeSocket();
            return false;
        }
 
        freeaddrinfo(res);
        listeningSockets.push_back(listen_fd);

        std::cout << "Server " << i << " listening on port " << server.port << " (fd=" << listen_fd << ")\n";
    }
    return true;
}

const std::vector<int>& SocketManager::getSockets() const {
    return listeningSockets;
}

void SocketManager::closeSocket() {
    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        if (listeningSockets[i] != -1) {
            close(listeningSockets[i]);
        }
    }
    listeningSockets.clear();
}

bool SocketManager::isServerSocket(int fd) const {
    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        if (fd == listeningSockets[i])
            return true;
    }
    return false;
}

void SocketManager::send408(int client_fd) {
    const char *timeout_respond = 
        "HTTP/1.1 408 Request Timeout\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    
    // Use MSG_DONTWAIT to ensure non-blocking send
    send(client_fd, timeout_respond, strlen(timeout_respond), MSG_DONTWAIT | MSG_NOSIGNAL);
}

void SocketManager::acceptNewClient(int readyServerFd, int epfd) {
    int connection_fd = accept(readyServerFd, NULL, NULL);
    if (connection_fd == -1)
        return;

    if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1) {
        close(connection_fd);
        return;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = connection_fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connection_fd, &ev) == -1) {
        close(connection_fd);
        return;
    }
    std::cout << "Accepted new client fd=" << connection_fd << std::endl;
}

void SocketManager::handleRequest(int readyServerFd, int epfd) {
    char buf[4096];

    ssize_t n = recv(readyServerFd, buf, sizeof(buf), 0);
    if (n <= 0) {
        close(readyServerFd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, readyServerFd, 0);
        requestBuffers.erase(readyServerFd);
        lastActivity.erase(readyServerFd);
        std::cout << "Closed client fd=" << readyServerFd << std::endl;
        return;
    }
    
    lastActivity[readyServerFd] = time(NULL);
    requestBuffers[readyServerFd].append(buf, n);
    size_t header_end = requestBuffers[readyServerFd].find("\r\n\r\n");
    if (header_end != std::string::npos) {
        // std::string &rawRequest = requestBuffers[readyServerFd];
        std::cout << "Full request from fd=" << readyServerFd
                  << ":\n" << requestBuffers[readyServerFd] << std::endl;

        ///TO-DO!
        //1. parse HTTP request
        // HttpRequest request = parseHttpRequest(rawRequest);

        //2. Takes the parsed request and decides what the server should reply:
        // std::string response = BuildResponse(request);

        //3. send the response
        //actually writes the reply to the network.
        // sendResponse(readyServerFd, response);

        requestBuffers[readyServerFd].clear();
    }
}

void SocketManager::handleTimeouts(int epfd) {
    time_t now = time(NULL);
    std::map<int, time_t>::iterator it = lastActivity.begin();

    while (it != lastActivity.end()) {
        int fd = it->first;
        std::string &buf = requestBuffers[fd];

        bool headersComplete = (buf.find("\r\n\r\n") != std::string::npos);
        if (!headersComplete && now - it->second >  CLIENT_TIMEOUT) {
            send408(fd);
            close(fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
            requestBuffers.erase(fd);
            std::map<int, time_t>::iterator eraseIt = it++;
            lastActivity.erase(eraseIt);
        }
        else
            ++it;
    }
}

void SocketManager::handleClients() {
    int epfd = epoll_create1(0);
    if (epfd == -1)
        throw std::runtime_error("Failed to create epoll instance");

    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        int listening_fd = listeningSockets[i];
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = listening_fd;

        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listening_fd, &event) == -1)
            throw std::runtime_error("Failed to add server socket to epoll");
    }
    std::vector<struct epoll_event> events(1024);
    while (true) {
        int n = epoll_wait(epfd, &events[0], events.size(), 1000);
        if (n == -1) {  
            if (errno == EINTR)
                continue;    
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < n; ++i) {
            int readyServerFd = events[i].data.fd;

            if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                std::cerr << "Closing fd " << readyServerFd << " due to EPOLLHUP/EPOLLERR" << std::endl;
                close(readyServerFd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, readyServerFd, 0);
                requestBuffers.erase(readyServerFd);
                lastActivity.erase(readyServerFd);
                continue;
            }

            if (isServerSocket(readyServerFd))
                acceptNewClient(readyServerFd, epfd);
            else if (events[i].events & EPOLLIN)
                handleRequest(readyServerFd, epfd);
        }
        handleTimeouts(epfd);
    }
}
