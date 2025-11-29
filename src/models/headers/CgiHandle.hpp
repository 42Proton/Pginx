#ifndef CGUTIHANDLE_HPP
#define CGUTIHANDLE_HPP


#include <exception>
#include <string>
#include <sstream>
#include <map>
#include <cstring>
#include <iomanip>
#include "BaseBlock.hpp"
#include "HttpRequest.hpp"
#include "requestContext.hpp"
#include "Server.hpp"
class HttpRequest;
class RequestContext;

class CgiHandle{
  private:
    CgiHandle();

  public:
  void buildCgiEnvironment(const HttpRequest& request,const RequestContext& ctx,const std::string& scriptPath,const std::string& serverPort,const std::string& clientIP,const std::string &serverName,std::map<std::string, std::string>& envVars);
    // std::string getCgiResponse(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData);
    void getInterpreterForScript(std::map<std::string, std::string> &cgiPassMap, const std::string &scriptPath, std::string &interpreterPath);
    void getDirectoryFromPath(const std::string &path, std::string &directoryPath);
    // void runCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData);
    // void terminateCgiProcess(pid_t pid);
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