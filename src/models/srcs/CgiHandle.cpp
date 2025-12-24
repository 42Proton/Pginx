#include "CgiHandle.hpp"
#include "HttpResponse.hpp"
#include <fcntl.h>
#include <sys/epoll.h>


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
    
    it = headers.find("cookie");
    if (it != headers.end())
        envVars["HTTP_COOKIE"] = it->second;
    
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

std::string CgiHandle::readCgiResponse(const std::string &inputData, int stdinPipe, int stdoutPipe, int epollFd, pid_t childPid) {
    int flags = fcntl(stdinPipe, F_GETFL, 0);
    fcntl(stdinPipe, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(stdoutPipe, F_GETFL, 0);
    fcntl(stdoutPipe, F_SETFL, flags | O_NONBLOCK);
    
    struct epoll_event event;
    
    event.events = EPOLLIN | EPOLLET; 
    event.data.fd = stdoutPipe;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, stdoutPipe, &event) == -1) {
        close(stdinPipe);
        close(stdoutPipe);
        throw CgiExecutionException();
    }
    
    bool needToWrite = !inputData.empty();
    size_t bytesWritten = 0;
    if (needToWrite) {
        event.events = EPOLLOUT | EPOLLET;
        event.data.fd = stdinPipe;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, stdinPipe, &event) == -1) {
            epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
            close(stdinPipe);
            close(stdoutPipe);
            throw CgiExecutionException();
        }
    } else {
        close(stdinPipe);
    }
    
    std::string output;
    char buffer[4096];
    time_t startTime = time(NULL);
    int timeout = 5; // 5 second timeout
    bool stdoutClosed = false;
    
    struct epoll_event events[2];
    
    while (!stdoutClosed) {
        // Check for timeout
        if (time(NULL) - startTime > timeout) {
            if (needToWrite) {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                close(stdinPipe);
            }
            epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
            close(stdoutPipe);
            kill(childPid, SIGKILL);
            waitpid(childPid, NULL, 0);
            throw CgiTimeoutException();
        }

        int nfds = epoll_wait(epollFd, events, 2, 100);

        if (nfds == -1) {
            if (needToWrite) {
                epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                close(stdinPipe);
            }
            epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
            close(stdoutPipe);
            throw CgiExecutionException();
        }
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == stdinPipe && (events[i].events & EPOLLOUT)) {
                ssize_t written = write(stdinPipe, inputData.c_str() + bytesWritten, 
                                       inputData.length() - bytesWritten);
                if (written > 0) {
                    bytesWritten += written;
                    if (bytesWritten >= inputData.length()) {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                        close(stdinPipe);
                        needToWrite = false;
                    }
                } else if (written == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                    close(stdinPipe);
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
                    close(stdoutPipe);
                    throw CgiExecutionException();
                }
            }
            
            if (events[i].data.fd == stdoutPipe && (events[i].events & EPOLLIN)) {
                while (true) {
                    ssize_t bytesRead = read(stdoutPipe, buffer, sizeof(buffer));
                    if (bytesRead > 0) {
                        output.append(buffer, bytesRead);
                    } else if (bytesRead == 0) {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
                        close(stdoutPipe);
                        stdoutClosed = true;
                        break;
                    } else if (bytesRead == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
                            close(stdoutPipe);
                            if (needToWrite) {
                                epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                                close(stdinPipe);
                            }
                            throw CgiExecutionException();
                        }
                    }
                }
            }
            
            if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                if (events[i].data.fd == stdoutPipe) {
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, stdoutPipe, NULL);
                    close(stdoutPipe);
                    stdoutClosed = true;
                }
                if (events[i].data.fd == stdinPipe && needToWrite) {
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
                    close(stdinPipe);
                    needToWrite = false;
                }
            }
        }
    }
    
    if (needToWrite) {
        epoll_ctl(epollFd, EPOLL_CTL_DEL, stdinPipe, NULL);
        close(stdinPipe);
    }
    
    return output;
}

void CgiHandle::getInterpreterForScript(const std::map<std::string, std::string> &cgiPassMap, const std::string &scriptPath, std::string &interpreterPath) {
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

void CgiHandle::parseCgiResponse(const std::string &cgiOutput, HttpResponse &res) {
    if (cgiOutput.empty()) {
        throw CgiInvalidResponseException();
    }
    
    std::istringstream responseStream(cgiOutput);
    std::string line;

    std::getline(responseStream, line);
    
    if (!line.empty() && line[line.length() - 1] == '\r')
        line = line.substr(0, line.length() - 1);
    
    bool hasStatusLine = false;
    if (line.find("HTTP/") == 0) {
        hasStatusLine = true;
        std::istringstream statusLineStream(line);
        std::string httpVersion;
        int statusCode;
        std::string statusMessage;
        statusLineStream >> httpVersion >> statusCode;
        std::getline(statusLineStream, statusMessage);
        res.setStatus(statusCode, statusMessage);
    } else {
        res.setStatus(200, "OK");
        if (!line.empty()) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 1);
                
                size_t start = headerValue.find_first_not_of(" \t\r\n");
                if (start != std::string::npos) {
                    headerValue = headerValue.substr(start);
                }
                size_t end = headerValue.find_last_not_of(" \t\r\n");
                if (end != std::string::npos) {
                    headerValue = headerValue.substr(0, end + 1);
                }
                
                if (headerName == "Set-Cookie") {
                    res.addSetCookieHeader(headerValue);
                } else {
                    res.setHeader(headerName, headerValue);
                }
            }
        }
    }

    while (std::getline(responseStream, line)) {
        if (!line.empty() && line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);
        
        if (line.empty())
            break;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            size_t start = headerValue.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) {
                headerValue = headerValue.substr(start);
            }
            size_t end = headerValue.find_last_not_of(" \t\r\n");
            if (end != std::string::npos) {
                headerValue = headerValue.substr(0, end + 1);
            }
            
            if (headerName == "Set-Cookie") {
                res.addSetCookieHeader(headerValue);
            } else {
                res.setHeader(headerName, headerValue);
            }
        }
    }

    std::string body;
    while (std::getline(responseStream, line)) {
        body += line + "\n";
    }
    res.setBody(body);
}

void CgiHandle::sendCgiOutputToClient(const std::string &cgiOutput, HttpResponse &res) {
    parseCgiResponse(cgiOutput, res);
    res.build();
}

std::string CgiHandle::executeCgiScript(const std::string &scriptPath, const std::map<std::string, std::string> &envVars, const std::string &inputData,
    const std::map<std::string, std::string> &cgiPassMap, int epollFd) {

    int stdinPipe[2];
    int stdoutPipe[2];
    std::string interpreterPath;

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
        
        if (epollFd != -1) {
            close(epollFd);
        }

        std::string scriptDir;
        getDirectoryFromPath(scriptPath, scriptDir);
        if (chdir(scriptDir.c_str()) == -1) {
            std::exit(1);
        }

        char **envp = convertMapToCharArray(envVars);
        getInterpreterForScript(cgiPassMap, scriptPath, interpreterPath);
        if (!interpreterPath.empty()) {
            char *argv[3];
            argv[0] = const_cast<char *>(interpreterPath.c_str());
            argv[1] = const_cast<char *>(scriptPath.c_str());
            argv[2] = NULL;
            execve(interpreterPath.c_str(), argv, envp);
            freeCharArray(envp, envVars.size());
            std::exit(1);
        }
        char *argv[2];
        argv[0] = const_cast<char *>(scriptPath.c_str());
        argv[1] = NULL;

        execve(scriptPath.c_str(), argv, envp);
        freeCharArray(envp, envVars.size());
        std::exit(1);
    } else {
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        
        std::string cgiOutput = readCgiResponse(inputData, stdinPipe[1], stdoutPipe[0], epollFd, pid);
        
        int status;
        pid_t result = waitpid(pid, &status, 0);

        if (result == -1) {
            throw CgiExecutionException();
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw CgiExecutionException();
        }
        return cgiOutput;
    }
}

void CgiHandle::buildCgiScript(const std::string &scriptPath, const RequestContext &ctx, HttpResponse &res, HttpRequest &request,
    sockaddr_in &clientAddr, int epollFd) {
    std::map<std::string, std::string> envVars;
    std::string serverName = ctx.server.getMatchingServerName(res.getHostHeader());
    u_int16_t serverPort = ctx.server.getServerPort(serverName);
    std::string clientIP = inet_ntoa(clientAddr.sin_addr);
    buildCgiEnvironment(request, ctx, scriptPath, serverPort, clientIP, serverName, envVars);
    try
    {
        std::string cgiOutput = executeCgiScript(scriptPath, envVars, request.getBody(), ctx.location->getCgiPassMap(), epollFd);
        sendCgiOutputToClient(cgiOutput, res);
    } 
    catch (const CgiTimeoutException& e) {
        std::cerr << "CGI Timeout: " << e.what() << '\n';
        res.setErrorFromContext(504, ctx); // Gateway Timeout
    }
    catch (const CgiInvalidResponseException& e) {
        std::cerr << "Invalid CGI Response: " << e.what() << '\n';
        res.setErrorFromContext(502, ctx); // Bad Gateway
    }
    catch (const CgiExecutionException& e) {
        std::cerr << "CGI Execution Error: " << e.what() << '\n';
        res.setErrorFromContext(500, ctx); // Internal Server Error
    }
    catch (const std::exception& e) {
        std::cerr << "Unknown error: " << e.what() << '\n';
        res.setErrorFromContext(500, ctx);
    }
    
}

