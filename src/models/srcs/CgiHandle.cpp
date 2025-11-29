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

// void CgiHandle::terminateCgiProcess(pid_t pid) {
//     // Implementation goes here
// }

std::string urlEncode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }
        escaped << '%' << std::setw(2) << int((unsigned char)c);
    }

    return escaped.str();
}


void buildCgiEnvironment(
    const HttpRequest& request,
    const RequestContext& ctx,
    const std::string& scriptPath,
    const std::string& serverPort,
    const std::string& clientIP,
    const std::string &serverName,
    std::map<std::string, std::string>& envVars)
{
    // 1. REQUEST_METHOD
    envVars["REQUEST_METHOD"] = request.getMethod();

    // 2. QUERY_STRING - rebuild from query map
    std::string queryString;
    const std::map<std::string, std::string>& queryParams = request.getQuery();
    for (std::map<std::string, std::string>::const_iterator it = queryParams.begin();
         it != queryParams.end(); ++it) {
        if (!queryString.empty()) queryString += "&";
        queryString += urlEncode(it->first) + "=" + urlEncode(it->second);
    }
    envVars["QUERY_STRING"] = queryString;
    
    // 3. PATH_INFO
    envVars["PATH_INFO"] = request.getPath();
    
    // 4. SCRIPT_NAME
    envVars["SCRIPT_NAME"] = scriptPath;
    
    // 5. SERVER_PROTOCOL
    envVars["SERVER_PROTOCOL"] = request.getVersion();
    
    // 6. Headers
    const std::map<std::string, std::string>& headers = request.getHeaders();
    
    std::map<std::string, std::string>::const_iterator it;
    
    it = headers.find("content-type");
    if (it != headers.end())
        envVars["CONTENT_TYPE"] = it->second;
    
    it = headers.find("content-length");
    if (it != headers.end())
        envVars["CONTENT_LENGTH"] = it->second;
    
    it = headers.find("host");
    if (it != headers.end())
        envVars["HTTP_HOST"] = it->second;
    
    // 7. Server info (from context)
    envVars["SERVER_NAME"] = serverName;
    envVars["SERVER_PORT"] = serverPort;
    envVars["REMOTE_ADDR"] = clientIP;
    
    // 8. Script-specific variables (REQUIRED by subject)
    envVars["SCRIPT_FILENAME"] = scriptPath;
    envVars["REDIRECT_STATUS"] = "200";      // Required for PHP-CGI security
    envVars["GATEWAY_INTERFACE"] = "CGI/1.1"; // CGI version
    
    envVars["DOCUMENT_ROOT"] = ctx.server.getRoot();

    // 10. All HTTP headers with HTTP_ prefix (REQUIRED: full request to CGI)
    for (it = headers.begin(); it != headers.end(); ++it) {
        std::string headerName = "HTTP_";
        for (size_t i = 0; i < it->first.length(); ++i) {
            char c = std::toupper(it->first[i]);
            headerName += (c == '-') ? '_' : c;
        }
        envVars[headerName] = it->second;
    }
}
// std::string CgiHandle::getCgiResponse(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData) {
//     // Implementation goes here
//     return "";
// }

void CgiHandle::getInterpreterForScript(std::map<std::string, std::string> &cgiPassMap, const std::string &scriptPath, std::string &interpreterPath) {
    size_t dotPos = scriptPath.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string extension = scriptPath.substr(dotPos);
        std::map<std::string, std::string>::const_iterator it = cgiPassMap.find(extension);
        if (it != cgiPassMap.end()) {
            interpreterPath = it->second;
            return;
        }
    }
    interpreterPath = "";
}

void CgiHandle::getDirectoryFromPath(const std::string &path, std::string &directoryPath) {
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        directoryPath = path.substr(0, pos);
    } else {
        directoryPath = ".";
    }
}

// void CgiHandle::runCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData) {
//     // Implementation goes here
// }