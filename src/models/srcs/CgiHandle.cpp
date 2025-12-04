#include "CgiHandle.hpp"
#include "HttpResponse.hpp"
#include <fcntl.h>


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


void CgiHandle::buildCgiEnvironment(
    const HttpRequest& request,
    const RequestContext& ctx,
    const std::string& scriptPath,
    u_int16_t serverPort,
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
    std::ostringstream portStream;
    portStream << serverPort;
    envVars["SERVER_PORT"] = portStream.str();
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

std::string CgiHandle::readCgiResponse(const std::string &inputData, int stdinPipe, int stdoutPipe) {
    // Write input data to stdin pipe
    if (!inputData.empty()) {
        write(stdinPipe, inputData.c_str(), inputData.length());
    }
    close(stdinPipe);
    
    // Read output from stdout pipe
    std::string output;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(stdoutPipe, buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytesRead);
    }
    close(stdoutPipe);
    
    return output;
}

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

void CgiHandle::sendCgiOutputToClient(const std::string &cgiOutput, HttpResponse &res) {
    res.setStatus(200, "OK");
    res.setBody(cgiOutput);
    std::stringstream lengthStream;
    lengthStream << cgiOutput.size();
    res.setHeader("Content-Length", lengthStream.str());
    res.setHeader("Content-Type", "text/html");
    res.build();
}

std::string CgiHandle::executeCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData) {
    
    int stdinPipe[2];
    int stdoutPipe[2];

    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
        throw CgiExecutionException();
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        throw CgiExecutionException();
    }
    else if (pid == 0) {
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);

        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        char **envp = convertMapToCharArray(envVars);

        char *argv[2];
        argv[0] = const_cast<char *>(scriptPath.c_str());
        argv[1] = NULL;

        execve(scriptPath.c_str(), argv, envp);
        freeCharArray(envp, envVars.size());
        exit(1);
    } else {
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        std::string cgiOutput = readCgiResponse(inputData, stdinPipe[1], stdoutPipe[0]);
        
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw CgiExecutionException();
        }
        return cgiOutput;
    }
}
void CgiHandle::buildCgiScript(const std::string &scriptPath, const RequestContext &ctx, HttpResponse &res, HttpRequest &request, sockaddr_in &clientAddr) {
    std::map<std::string, std::string> envVars;
    std::string serverName = ctx.server.getMatchingServerName(res.getHostHeader());
    u_int16_t serverPort = ctx.server.getServerPort(serverName);
    std::string clientIP = inet_ntoa(clientAddr.sin_addr);
    buildCgiEnvironment(request, ctx, scriptPath, serverPort, clientIP, serverName, envVars);
    try
    {
        std::string cgiOutput = executeCgiScript(scriptPath, envVars, request.getBody());
        sendCgiOutputToClient(cgiOutput, res);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        res.setErrorFromContext(500, ctx);
    }
    
}

