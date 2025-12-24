#include "SocketManager.hpp"
#include "Server.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "requestContext.hpp"
#include "ResourceGuards.hpp"
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

// the parentheses () mean default construction.
SocketManager::SocketManager()
    : listeningSockets(),
      requestBuffers(),
      lastActivity(),
      sendBuffers(),
      serverList(),
      httpParser(new HttpParser()),
      responseBuilder(new HttpResponse())
{
}

// Add this setter to initialize the server list
void SocketManager::setServers(const std::vector<Server> &servers)
{
    serverList = servers;
}

SocketManager::~SocketManager()
{
    closeSocket();
    // httpParser and responseBuilder auto-deleted by std::auto_ptr
}

std::string initToString(int n)
{
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

std::vector<ServerSocketInfo> convertServersToSocketInfo(const std::vector<Server> &servers)
{
    std::vector<ServerSocketInfo> socketInfos;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const Server &server = servers[i];
        const std::vector<ListenCtx> &listens = server.getListens();
        const std::vector<std::string> &serverNames = server.getServerNames();

        std::string serverName = "";
        if (!serverNames.empty())
        {
            serverName = serverNames[0];
        }
        for (size_t j = 0; j < listens.size(); ++j)
        {
            const ListenCtx &listen = listens[j];
            ServerSocketInfo info(listen.addr, initToString(listen.port), serverName);
            socketInfos.push_back(info);
        }
    }
    return socketInfos;
}

bool SocketManager::initSockets(const std::vector<ServerSocketInfo> &servers)
{
    std::map<std::string, int> existingSockets;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerSocketInfo &server = servers[i];
        std::string key = server.host + ":" + server.port;

        if (existingSockets.count(key))
        {
            std::cout << "Reusing existing socket for " << key
                      << " (fd=" << existingSockets[key] << ")" << std::endl;
            continue;
        }

        struct addrinfo hints;
        struct addrinfo *res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        const char *bindHost;
        if (server.host.empty())
            bindHost = "0.0.0.0";
        else
            bindHost = server.host.c_str();

        if (getaddrinfo(bindHost, server.port.c_str(), &hints, &res) != 0)
        {
            std::cerr << "getaddrinfo failed for " << key << std::endl;
            closeSocket();
            continue;
        }

        int listen_fd = -1;
        struct addrinfo *p;
        for (p = res; p != NULL; p = p->ai_next)
        {
            SocketGuard socketGuard(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
            if (!socketGuard.isValid())
                continue;

            int opt = 1;
            setsockopt(socketGuard.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            if (bind(socketGuard.get(), p->ai_addr, p->ai_addrlen) == 0)
            {
                if (listen(socketGuard.get(), 10) == -1)
                {
                    std::cerr << "listen failed for " << key << std::endl;
                    // SocketGuard auto-closes on continue
                }
                else
                {
                    listen_fd = socketGuard.release(); // Success - transfer ownership
                    break;
                }
            }
            // SocketGuard auto-closes on loop iteration if bind failed
        }
        freeaddrinfo(res);

        if (listen_fd == -1)
        {
            std::cerr << "Failed to bind any address for " << key << std::endl;
            closeSocket();
            continue;
        }

        listeningSockets.push_back(listen_fd);
        existingSockets[key] = listen_fd;

        std::cout << "Server listening on " << key
                  << " (fd=" << listen_fd << ")" << std::endl;
    }
    if (listeningSockets.empty())
    {
        std::cerr << "No sockets were successfully initialized." << std::endl;
        return false;
    }
    return true;
}

const std::vector<int> &SocketManager::getSockets() const
{
    return listeningSockets;
}

void SocketManager::closeSocket()
{
    for (size_t i = 0; i < listeningSockets.size(); ++i)
    {
        if (listeningSockets[i] != -1)
        {
            close(listeningSockets[i]);
        }
    }
    listeningSockets.clear();
}

bool SocketManager::isServerSocket(int fd) const
{
    for (size_t i = 0; i < listeningSockets.size(); ++i)
    {
        if (fd == listeningSockets[i])
            return true;
    }
    return false;
}

void SocketManager::acceptNewClient(int readyServerFd, int epfd, sockaddr_in &clientAddr, socklen_t &clientLen)
{
    SocketGuard connectionGuard(accept(readyServerFd, (sockaddr *)&clientAddr, &clientLen));
    if (!connectionGuard.isValid())
        return;

    if (fcntl(connectionGuard.get(), F_SETFL, O_NONBLOCK) == -1)
    {
        return; // SocketGuard auto-closes
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = connectionGuard.get();

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connectionGuard.get(), &ev) == -1)
    {
        return; // SocketGuard auto-closes
    }
    std::cout << "Accepted new client fd=" << connectionGuard.get() << std::endl;
    connectionGuard.release(); // Success - epoll now manages the FD
}

// Checks
bool SocketManager::isRequestTooLarge(int fd)
{
    return requestBuffers[fd].size() > MAX_REQUEST_SIZE;
}

bool SocketManager::isHeaderTooLarge(int fd)
{
    size_t header_end = requestBuffers[fd].find("\r\n\r\n");
    if (header_end == std::string::npos)
        return requestBuffers[fd].size() > MAX_HEADER_SIZE;
    return false;
}

bool SocketManager::isRequestLineMalformed(int fd)
{
    size_t line_end = requestBuffers[fd].find("\r\n");
    if (line_end == std::string::npos)
        return false;

    std::string request_line = requestBuffers[fd].substr(0, line_end);
    size_t first_space = request_line.find(' ');
    size_t last_space = request_line.rfind(' ');

    if (first_space == std::string::npos || last_space == std::string::npos || first_space == last_space)
        return true;

    std::string method = request_line.substr(0, first_space);
    std::string version = request_line.substr(last_space + 1);

    if (version != "HTTP/1.0" && version != "HTTP/1.1")
        return true;

    return false;
}

bool SocketManager::hasNonPrintableCharacters(int fd)
{
    size_t line_end = requestBuffers[fd].find("\r\n");
    if (line_end == std::string::npos)
        return false;

    std::string line = requestBuffers[fd].substr(0, line_end);
    for (size_t i = 0; i < line.size(); ++i)
    {
        if (!isprint(line[i]) && !isspace(line[i]))
            return true;
    }
    return false;
}

bool SocketManager::isBodyTooLarge(int fd)
{
    size_t header_end = requestBuffers[fd].find("\r\n\r\n");
    if (header_end == std::string::npos)
        return false;

    std::string headers = requestBuffers[fd].substr(0, header_end);

    // Check if Transfer-Encoding: chunked is present
    // For chunked encoding, size validation happens after un-chunking
    size_t te_pos = headers.find("Transfer-Encoding:");
    if (te_pos != std::string::npos)
    {
        std::string te_value = headers.substr(te_pos + 18);
        size_t te_end = te_value.find("\r\n");
        if (te_end != std::string::npos)
            te_value = te_value.substr(0, te_end);
        
        // Convert to lowercase for comparison
        for (size_t i = 0; i < te_value.length(); ++i)
            te_value[i] = std::tolower(te_value[i]);
        
        if (te_value.find("chunked") != std::string::npos)
        {
            // Skip validation for chunked requests
            // Size will be validated after un-chunking
            return false;
        }
    }

    // Find Content-Length
    size_t content_length = 0;
    size_t cl_pos = headers.find("Content-Length:");
    if (cl_pos != std::string::npos)
    {
        std::string cl_str = headers.substr(cl_pos + 15);
        std::istringstream iss(cl_str);
        iss >> content_length;
    }

    if (content_length > MAX_BODY_SIZE)
        return true;

    // Check if body already received exceeds content_length or MAX_BODY_SIZE
    size_t body_received = requestBuffers[fd].size() - (header_end + 4);
    if (body_received > content_length || body_received > MAX_BODY_SIZE)
        return true;

    return false;
}

bool SocketManager::hasInvalidPercentEncoding(int fd)
{
    size_t line_end = requestBuffers[fd].find("\r\n");
    if (line_end == std::string::npos)
        return false;

    std::string line = requestBuffers[fd].substr(0, line_end);
    size_t first_space = line.find(' ');
    size_t last_space = line.rfind(' ');
    if (first_space == std::string::npos || last_space == std::string::npos || first_space == last_space)
        return false;

    std::string path = line.substr(first_space + 1, last_space - first_space - 1);

    for (size_t i = 0; i < path.size(); ++i)
    {
        if (path[i] == '%')
        {
            if (i + 2 >= path.size() || !isxdigit(path[i + 1]) || !isxdigit(path[i + 2]))
                return true;
            i += 2;
        }
    }
    return false;
}

void SocketManager::sendHttpError(int fd, const std::string &status, int epfd)
{
    int code = atoi(status.c_str());

    Server &server = selectServerForClient(fd);
    RequestContext ctx(server, NULL);

    std::string body;

    try
    {
        body = ctx.getErrorPageContent(code);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error while loading error page: " << e.what() << std::endl;

        std::ostringstream fallback;
        fallback << "<html><body><h1>Error " << code << "</h1></body></html>";
        body = fallback.str();
    }

    std::ostringstream res;
    res << "HTTP/1.0 " << status << "\r\n"
        << "Content-Type: text/html\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;

    sendBuffers[fd] = res.str();

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

Server &SocketManager::selectServerForClient(int clientFd)
{

    struct sockaddr_in serverAddr;
    socklen_t addrlen = sizeof(serverAddr);
    if (getsockname(clientFd, (struct sockaddr *)&serverAddr, &addrlen) == -1)
    {
        return serverList[0];
    }
    std::string serverIP = inet_ntoa(serverAddr.sin_addr);
    u_int16_t serverPort = ntohs(serverAddr.sin_port);

    for (size_t i = 0; i < serverList.size(); ++i)
    {
        const std::vector<ListenCtx> &listens = serverList[i].getListens();
        for (size_t j = 0; j < listens.size(); ++j)
        {
            if (listens[j].port == serverPort &&
                (listens[j].addr == "0.0.0.0" || listens[j].addr == serverIP))
            {
                return serverList[i];
            }
        }
    }
    return serverList[0];
}

// dummy full until omran finishes the parsing
HttpRequest *SocketManager::fillRequest(const std::string &rawRequest, Server &server)
{
    // std::cout << "=== Raw request ===\n" << rawRequest << "\n=== End ===" << std::endl;

    std::istringstream stream(rawRequest);
    std::string requestLine;

    // Read the first line: "METHOD /path HTTP/1.1"
    if (!std::getline(stream, requestLine))
        return 0; // Malformed or empty

    // Remove trailing '\r'
    if (!requestLine.empty() && requestLine[requestLine.size() - 1] == '\r')
        requestLine.erase(requestLine.size() - 1);

    std::istringstream lineStream(requestLine);
    std::string method, path, version;
    lineStream >> method >> path >> version;
    if (method.empty() || path.empty() || version.empty())
    return 0; // Malformed request line
    
    // Parse query string to get clean path for location matching
    std::string cleanPath;
    std::map<std::string, std::string> query;
    HttpRequest::parseQuery(path, cleanPath, query);
    
    // Find matching location for this path
    const LocationConfig *location = server.findLocation(cleanPath);
    
    // Create RequestContext with server and location
    RequestContext ctx(server, location);

    // Create appropriate HttpRequest subclass
    HttpRequest *request = makeRequestByMethod(method, ctx);
    if (!request)
        return 0; // Unsupported method

    request->setMethod(method);
    request->setVersion(version);
    request->setPath(cleanPath);
    request->setQuery(query);

    // Parse headers until empty line (CRLF)
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            break;

        std::string key, value;
        if (HttpRequest::parseHeaderLine(line, key, value))
            request->addHeader(key, value);
    }

    // Parse the body (if present)
    std::string body, chunk;
    while (std::getline(stream, chunk))
    {
        if (!chunk.empty() && chunk[chunk.size() - 1] == '\r')
            chunk.erase(chunk.size() - 1);
        body += chunk;
        body += "\n";
    }

    if (!body.empty())
    {
        if (request->isChunked())
        {
            std::string unchunkedBody;
            size_t pos = 0;
            
            while (pos < body.length())
            {
                // Find chunk size line
                size_t lineEnd = body.find('\n', pos);
                if (lineEnd == std::string::npos)
                    break;
                
                std::string sizeLine = body.substr(pos, lineEnd - pos);
                while (!sizeLine.empty() && (sizeLine[sizeLine.length() - 1] == '\r' || 
                                             sizeLine[sizeLine.length() - 1] == ' '))
                    sizeLine.erase(sizeLine.length() - 1);
                
                size_t semicolon = sizeLine.find(';');
                if (semicolon != std::string::npos)
                    sizeLine = sizeLine.substr(0, semicolon);
                
                // Parse hex chunk size
                std::istringstream hexStream(sizeLine);
                unsigned long chunkSize;
                hexStream >> std::hex >> chunkSize;
                
                if (hexStream.fail())
                    break;
                pos = lineEnd + 1;
                // Check for terminating chunk
                if (chunkSize == 0)
                    break;
                
                // Extract chunk data
                if (pos + chunkSize <= body.length())
                {
                    unchunkedBody.append(body.substr(pos, chunkSize));
                    pos += chunkSize;
                    
                    // Skip trailing newline after chunk data
                    if (pos < body.length() && body[pos] == '\n')
                        pos++;
                }
                else
                {
                    break;
                }
            }
            
            request->appendBody(unchunkedBody);
        }
        else
        {
            request->appendBody(body);
        }
    }

    return request;
}

void SocketManager::processFullRequest(int readyServerFd, int epfd, const std::string &rawRequest, sockaddr_in &clientAddr)
{
    Server &myServer = selectServerForClient(readyServerFd);

    RequestGuard request(fillRequest(rawRequest, myServer));
    if (!request.isValid())
    {
        sendHttpError(readyServerFd, "400 Bad Request", epfd);
        requestBuffers[readyServerFd].clear();
        return;
    }

    // Validate the request before handling it
    // std::string validationError;
    // if (!request->validate(validationError))
    // {
    //     // Request validation failed
    //     HttpResponse res;
    //     res.setError(400, "Bad Request");
    //     res.setBody("<h1>400 Bad Request</h1><p>" + validationError + "</p>");
    //     res.setVersion("HTTP/1.0");
    //     sendBuffers[readyServerFd] = res.build();

    //     struct epoll_event ev;
    //     ev.events = EPOLLIN | EPOLLOUT;
    //     ev.data.fd = readyServerFd;
    //     epoll_ctl(epfd, EPOLL_CTL_MOD, readyServerFd, &ev);

    //     requestBuffers[readyServerFd].clear();
    //     return; // RequestGuard automatically deletes on scope exit
    // }

    HttpResponse res;
    request->handle(res, clientAddr, epfd);
    res.setVersion("HTTP/1.0");
    sendBuffers[readyServerFd] = res.build();

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = readyServerFd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, readyServerFd, &ev);

    requestBuffers[readyServerFd].clear();
    // RequestGuard automatically deletes request when function exits
}

bool SocketManager::isRequestMalformed(int fd)
{
    return isRequestLineMalformed(fd) || hasNonPrintableCharacters(fd) || hasInvalidPercentEncoding(fd);
}

bool SocketManager::validateRequestSize(int fd, int epfd)
{
    size_t header_end = requestBuffers[fd].find("\r\n\r\n");

    if (header_end == std::string::npos && requestBuffers[fd].size() > MAX_HEADER_SIZE)
    {
        sendHttpError(fd, "431 Request Header Fields Too Large", epfd);
        return false;
    }

    if (header_end != std::string::npos)
    {
        if (requestBuffers[fd].size() > MAX_REQUEST_SIZE || isBodyTooLarge(fd))
        {
            sendHttpError(fd, "413 Payload Too Large", epfd);
            requestBuffers[fd].clear();
            return false;
        }
    }

    return true;
}

void SocketManager::handleRequest(int readyServerFd, int epfd, sockaddr_in &clientAddr)
{
    char buf[4096];

    ssize_t n = recv(readyServerFd, buf, sizeof(buf), 0);
    if (n <= 0)
    {
        close(readyServerFd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, readyServerFd, 0);
        requestBuffers.erase(readyServerFd);
        lastActivity.erase(readyServerFd);
        std::cout << "Closed client fd=" << readyServerFd << std::endl;
        return;
    }

    lastActivity[readyServerFd] = time(NULL);
    requestBuffers[readyServerFd].append(buf, n);

    // Early malformed request validation
    if (isRequestMalformed(readyServerFd))
    {
        sendHttpError(readyServerFd, "400 Bad Request", epfd);
        requestBuffers[readyServerFd].clear();
        return;
    }

    // Request size validation
    if (!validateRequestSize(readyServerFd, epfd))
        return;

    // If everything looks good and headers are complete, process request
    size_t header_end = requestBuffers[readyServerFd].find("\r\n\r\n");
    if (header_end != std::string::npos)
    {
        processFullRequest(readyServerFd, epfd, requestBuffers[readyServerFd], clientAddr);
        requestBuffers[readyServerFd].clear();
    }
}

void SocketManager::handleTimeouts(int epfd)
{
    time_t now = time(NULL);
    std::map<int, time_t>::iterator it = lastActivity.begin();

    while (it != lastActivity.end())
    {
        int fd = it->first;
        std::string &buf = requestBuffers[fd];

        bool headersComplete = (buf.find("\r\n\r\n") != std::string::npos);

        if (!headersComplete && now - it->second > CLIENT_TIMEOUT)
        {
            sendHttpError(fd, "408 Request Timeout", epfd);
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLOUT;
            ev.data.fd = fd;
            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);

            ++it;
        }
        else
            ++it;
    }
}

void SocketManager::sendBuffer(int fd, int epfd)
{
    std::map<int, std::string>::iterator it = sendBuffers.find(fd);
    if (it == sendBuffers.end())
        return;

    ssize_t sent = send(fd, it->second.c_str(), it->second.size(), MSG_NOSIGNAL | MSG_DONTWAIT);

    if (sent > 0)
    {
        it->second.erase(0, sent);
    }

    if (it->second.empty() || sent <= 0)
    {
        close(fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
        requestBuffers.erase(fd);
        lastActivity.erase(fd);
        sendBuffers.erase(fd);
    }
}

void SocketManager::handleClients()
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    EpollGuard epollGuard(epoll_create1(EPOLL_DEFAULT));
    if (!epollGuard.isValid())
        throw std::runtime_error("Failed to create epoll instance");

    int epfd = epollGuard.get();

    for (size_t i = 0; i < listeningSockets.size(); ++i)
    {
        int listening_fd = listeningSockets[i];
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = listening_fd;

        if (epoll_ctl(epfd, EPOLL_CTL_ADD, listening_fd, &event) == -1)
            throw std::runtime_error("Failed to add server socket to epoll");
    }
    std::vector<struct epoll_event> events(1024);
    while (true)
    {
        int n = epoll_wait(epfd, &events[0], events.size(), 1000);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < n; ++i)
        {
            int readyServerFd = events[i].data.fd;

            if (events[i].events & (EPOLLHUP | EPOLLERR))
            {
                std::cerr << "Closing fd " << readyServerFd << " due to EPOLLHUP/EPOLLERR" << std::endl;
                close(readyServerFd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, readyServerFd, 0);
                requestBuffers.erase(readyServerFd);
                lastActivity.erase(readyServerFd);
                continue;
            }
            if (isServerSocket(readyServerFd))
                acceptNewClient(readyServerFd, epfd, clientAddr, clientLen);
            else if (events[i].events & EPOLLIN)
                handleRequest(readyServerFd, epfd, clientAddr);
            if (events[i].events & EPOLLOUT)
                sendBuffer(readyServerFd, epfd);
        }
        handleTimeouts(epfd);
    }
}
