#ifndef CGUTIHANDLE_HPP
#define CGUTIHANDLE_HPP


#include <exception>
#include <string>
#include <map>
#include <cstring>

class CgiHandle {
  private:
    CgiHandle();

  public:
    void buildCgiEnvironment(const std::map<std::string, std::string> &requestHeaders, const std::string &scriptPath, std::map<std::string, std::string> &envVars);
    std::string getCgiResponse(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData);
    void runCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData);
    void terminateCgiProcess(pid_t pid);
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