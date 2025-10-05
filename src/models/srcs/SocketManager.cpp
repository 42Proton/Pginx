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
                return false;
        }

        int listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listen_fd == -1) {
            std::cerr << "socket creation failed for server " << i << "\n";
            freeaddrinfo(res);
            return false;
        }

        int opt = 1;
        // setsockopt → This system call configures options on a socket (listen_fd).
        //SOL_SOCKET → This tells the kernel that the option belongs to the socket layer,
        //not to a specific protocol like TCP or UDP.
        
        //SO_REUSEADDR:
        //When a server binds to a port (say port 8080), and then closes,
        //the OS keeps that port in a state called TIME_WAIT for a short period.
        //During TIME_WAIT, if you immediately restart your server, bind() may fail
        //Setting SO_REUSEADDR allows your socket to reuse that port immediately, 
        //even if it’s still in TIME_WAIT.
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

// Accept a new client and register it to epoll
void SocketManager::acceptNewClient(int readyServerFd, int epoll_fd) {
    int connection_fd = accept(readyServerFd, NULL, NULL);
    if (connection_fd == -1)
        return;

    // Make client socket non-blocking
    if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1) {
        close(connection_fd);
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = connection_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &ev) == -1) {
        close(connection_fd);
        return;
    }

    std::cout << "Accepted new client fd=" << connection_fd << std::endl;
}

// Handle data from a client
void SocketManager::handleRequest(int client_fd, int epoll_fd) {
    char buf[4096];
    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, 0);
        requestBuffers.erase(client_fd);
        std::cout << "Closed client fd=" << client_fd << std::endl;
        return;
    }
    // Append data to per-client buffer
    requestBuffers[client_fd].append(buf, n);
    // Check if headers are complete
    size_t header_end = requestBuffers[client_fd].find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::cout << "Full request from fd=" << client_fd
                  << ":\n" << requestBuffers[client_fd] << std::endl;

        // TODO: parseHttpRequest(requestBuffers[client_fd]);  //emaran
        // TODO: sendResponse(client_fd, ...);
        requestBuffers[client_fd].clear();
    }
}
// Main loop handling multiple clients
void SocketManager::handleClients() {
    //creates an epoll instance like a continer to hold all sockets you
    //want to monitor
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        throw std::runtime_error("Failed to create epoll instance");

    // Register all server sockets
    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        int listening_fd = listeningSockets[i];
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = listening_fd;

        //epoll_ctl() is the function where you tell epoll what action
        //you want to monitor on a specific socket.
        //EPOLL_CTL_ADD → we are adding a new socket to the watch list
        //server_fd → the socket we want to monitor (listening socket)
        //&event → what events we care about (EPOLLIN = ready to read)

        //POLL_CTL_ADD = add the socket to epoll’s internal watch list
        //epoll maintains it's own list of fds internally inside the kernal,
        //you do not see this list directly in your code it's managed by the OS
        //each time you call epoll_ctl the kernal stores that fd and the events 
        //it its internal data structures

        /*
            “Hey kernel — please start watching this fd for the event type I specify (EPOLLIN).
            If that event ever happens in the future, mark it as ready so epoll_wait() can return it.”
        */
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listening_fd, &event) == -1)
            throw std::runtime_error("Failed to add server socket to epoll");
    }

    //1024 the max number of ready fds epoll_wait() can report in a single call
    std::vector<struct epoll_event> events(1024);
    while (true) {
        //epoll_wait() reads from the kernal's internal data structure and returning 
        //only the sockets that are ready
        //epoll_wait(epoll_fd, events, max_events, timeout);
        // -1 -> This ensures the server sleeps until something happens → very CPU-efficient.
        
        //how epoll use events vector?
        //1. Kernel looks at its internal watch list.
        //2. Finds sockets ready for the events you asked for (EPOLLIN, EPOLLOUT, etc.).
        //3. Copies their fd + event flags into the events array you provided.
        //4. Returns n → number of entries it wrote into your array.

        //epoll_wait as soon as:
        //1. at least one fd becomes ready
        //2. timeout expires (if set one)
        //3. the call is interrupted by a signal
        int n = epoll_wait(epoll_fd, &events[0], events.size(), -1);
        if (n == -1)
            throw std::runtime_error("epoll_wait failed");

        for (int i = 0; i < n; ++i) {
            int readyServerFd = events[i].data.fd;

            if (isServerSocket(readyServerFd))
                acceptNewClient(readyServerFd, epoll_fd);
            else
                handleRequest(readyServerFd, epoll_fd);
        }
    }
    close(epoll_fd);
}