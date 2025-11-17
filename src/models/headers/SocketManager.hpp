#ifndef SOCKETMANAGER_HPP
#define SOCKETMANAGER_HPP

#include <arpa/inet.h>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>

// Forward declarations
class HttpParser;
class HttpRequest;
class HttpResponse;
class Server;
#define EPOLL_DEFAULT 0
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
  // Server list for multi-server support
  std::vector<Server> serverList;

  // HTTP processing components (RAII auto-cleanup)
  std::auto_ptr<HttpParser> httpParser;
  std::auto_ptr<HttpResponse> responseBuilder;

public:
  SocketManager();
  ~SocketManager();

  // Server management
  void setServers(const std::vector<Server> &servers);
  Server &selectServerForClient(int clientFd);

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
  bool isRequestMalformed(int fd);
  bool hasNonPrintableCharacters(int fd);
  bool validateRequestSize(int fd, int epfd);
  void sendHttpError(int fd, const std::string &status, int epfd);
  void sendHttpErrorWithCustomPage(int fd, int statusCode, const std::string &statusText, const Server &server, int epfd);
  bool isBodyTooLarge(int fd);
  bool validateRequest(int fd, int epfd);
  bool hasInvalidPercentEncoding(int fd);

  void sendHttpResponse(int fd, int epfd, const HttpResponse &res);
  HttpRequest *fillRequest(const std::string &rawRequest, Server &server);
  void processFullRequest(int readyServerFd, int epfd, const std::string &rawRequest);
};

#endif