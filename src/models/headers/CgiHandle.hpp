#ifndef CGUTIHANDLE_HPP
#define CGUTIHANDLE_HPP


#include <exception>
#include <string>
#include <sstream>
#include <map>
#include <cstring>
#include <iomanip>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "BaseBlock.hpp"
#include "HttpRequest.hpp"
#include "requestContext.hpp"
#include "Server.hpp"
class HttpRequest;
class RequestContext;

class CgiHandle{

  public:
  CgiHandle();
    void buildCgiEnvironment(const HttpRequest& request,const RequestContext& ctx,const std::string& scriptPath, u_int16_t serverPort,const std::string& clientIP,const std::string &serverName,std::map<std::string, std::string>& envVars);
    std::string readCgiResponse(const std::string &inputData, int stdinPipe, int stdoutPipe);
    void getInterpreterForScript(const std::map<std::string, std::string> &cgiPassMap, const std::string &scriptPath, std::string &interpreterPath);
    // void getDirectoryFromPath(const std::string &path, std::string &directoryPath);
    void buildCgiScript(const std::string &scriptPath, const RequestContext &ctx, HttpResponse &res, HttpRequest &request, sockaddr_in &clientAddr, int epollFd);
    std::string executeCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData, const std::map<std::string, std::string> &cgiPassMap, int epollFd);
    void sendCgiOutputToClient(const std::string &cgiOutput, HttpResponse &res);
    void parseCgiResponse(const std::string &cgiOutput, HttpResponse &res);


    class CgiExecutionException : public std::exception {
      public:
        const char *what() const throw();
    };

    class CgiTimeoutException : public std::exception {
      public:
        const char *what() const throw();
    };

    class CgiInvalidResponseException : public std::exception {
      public:
        const char *what() const throw();
    };
    ~CgiHandle();
};




#endif