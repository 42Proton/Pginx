#include "CgiHandle.hpp"

const char *CgiHandle::CgiExecutionException::what() const throw() {
    return "CGI Execution Failed";
}

const char *CgiHandle::CgiTimeoutException::what() const throw() {
    return "CGI Timeout";
}

const char *CgiHandle::CgiInvalidResponseException::what() const throw() {
    return "CGI Invalid Response";
}

CgiHandle::CgiHandle() {}

CgiHandle::~CgiHandle() {}

char **convertMapToCharArray(const std::map<std::string, std::string> &envVars) {
    char **envp = new char *[envVars.size() + 1];
    size_t index = 0;
    for (std::map<std::string, std::string>::const_iterator it = envVars.begin(); it != envVars.end(); ++it, ++index) {
        std::string envEntry = it->first + "=" + it->second;
        envp[index] = new char[envEntry.size() + 1];
        std::strcpy(envp[index], envEntry.c_str());
    }
    envp[index] = NULL;
    return envp;
}

void freeCharArray(char **envp, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        delete[] envp[i];
    }
    delete[] envp;
}

void CgiHandle::terminateCgiProcess(pid_t pid) {
    // Implementation goes here
}

void CgiHandle::buildCgiEnvironment(const std::map<std::string, std::string> &requestHeaders, const std::string &scriptPath, std::map<std::string, std::string> &envVars) {
    // Implementation goes here
}

std::string CgiHandle::getCgiResponse(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData) {
    // Implementation goes here
    return "";
}


void CgiHandle::runCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData) {
    // Implementation goes here
}