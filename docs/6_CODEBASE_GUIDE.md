# 🗂️ Codebase Guide

**Deep Dive into the Pginx Source Code**

This guide walks through the actual code files, explaining what each component does and how they work together.

---

## 📋 Table of Contents

1. [Project Structure](#project-structure)
2. [Entry Point - main.cpp](#entry-point---maincpp)
3. [Configuration System](#configuration-system)
4. [Network Layer - SocketManager](#network-layer---socketmanager)
5. [HTTP Parsing - HttpParser](#http-parsing---httpparser)
6. [Request Handling - HttpRequest](#request-handling---httprequest)
7. [Response Building - HttpResponse](#response-building---httpresponse)
8. [Utility Functions](#utility-functions)
9. [Build System - Makefile](#build-system---makefile)
10. [Testing Infrastructure](#testing-infrastructure)

---

## 📁 Project Structure

```
Pginx/
├── src/
│   ├── main.cpp                      # Entry point
│   ├── utils.cpp                     # Utility functions
│   ├── extCheck.cpp                  # External checks
│   └── models/
│       ├── headers/                  # All header files
│       │   ├── BaseBlock.hpp         # Base config class
│       │   ├── Container.hpp         # Holds all servers
│       │   ├── Server.hpp            # Server configuration
│       │   ├── LocationConfig.hpp    # Location block config
│       │   ├── SocketManager.hpp     # Network I/O manager
│       │   ├── HttpParser.hpp        # HTTP parser
│       │   ├── HttpRequest.hpp       # Request representation
│       │   ├── HttpResponse.hpp      # Response builder
│       │   ├── HttpUtils.hpp         # HTTP utilities
│       │   ├── parser.hpp            # Config file parser
│       │   └── requestContext.hpp    # Request context
│       └── srcs/                     # Implementation files
│           ├── BaseBlock.cpp
│           ├── Container.cpp
│           ├── Server.cpp
│           ├── LocationConfig.cpp
│           ├── SocketManager.cpp
│           ├── HttpParser.cpp
│           ├── HttpRequest.cpp
│           ├── HttpResponse.cpp
│           ├── HttpUtils.cpp
│           ├── parser.cpp
│           ├── lexer.cpp
│           └── readFile.cpp
├── includes/
│   ├── defaults.hpp                  # Default values
│   └── utils.hpp                     # Common utilities
├── config/                           # Configuration files
│   ├── webserv.conf
│   ├── default.conf
│   ├── complex_test.conf
│   └── edge_test.conf
├── www/                              # Web content
│   ├── index.html
│   ├── upload_form.html
│   └── error_pages/
│       ├── 404.html
│       ├── 500.html
│       └── ...
├── Tests/                            # Test scripts
│   ├── run_all_tests.sh
│   ├── core_tests.sh
│   ├── post_tests.sh
│   └── delete_tests.sh
├── docs/                             # Documentation (you're here!)
├── Makefile                          # Build system
└── webserv                           # Compiled executable
```

---

## 🚀 Entry Point - main.cpp

**Path:** `src/main.cpp`

### What It Does

The entry point initializes everything and starts the server.

```cpp
int main(int argc, char **argv) {
    // 1. Validate arguments
    if (argc != 2) {
        std::cerr << "Provide a configuration file!" << std::endl;
        return 1;
    }

    try {
        // 2. Initialize and validate
        initValidation(argc, argv);

        // 3. Read configuration file
        std::string content = readFile(argv[1]);

        // 4. Tokenize (lexer)
        std::vector<Token> tokens = lexer(content);

        // 5. Validate tokens
        checks(tokens);

        // 6. Parse into Container (holds all servers)
        Container container = parser(tokens);

        // 7. Convert servers to socket information
        std::vector<ServerSocketInfo> socketInfos =
            convertServersToSocketInfo(container.getServers());

        // 8. Create socket manager
        SocketManager socketManager;
        socketManager.setServers(container.getServers());

        // 9. Initialize sockets (bind, listen)
        if (!socketManager.initSockets(socketInfos)) {
            std::cerr << "Failed to initialize sockets!" << std::endl;
            return 1;
        }

        // 10. Start main event loop
        std::cout << "Server initialized. Waiting for clients..." << std::endl;
        socketManager.handleClients();  // Runs forever

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Flow Diagram

```
main()
  ↓
Read config file
  ↓
Lexer (tokenize) → tokens
  ↓
Parser → Container → [Server, Server, ...]
  ↓
SocketManager
  ↓
initSockets() → bind/listen on ports
  ↓
handleClients() → INFINITE LOOP
                  ↓
                epoll_wait()
                  ↓
            Handle events
```

---

## ⚙️ Configuration System

The configuration system parses NGINX-style config files.

### Configuration File Format

**Example:** `config/webserv.conf`

```nginx
http {
    server {
        listen 8002;
        server_name localhost;

        root ./www;
        index index.html;
        error_page 404 error_pages/404.html;

        location / {
            allow_methods DELETE POST GET;
            autoindex off;
        }
    }
}
```

### Class Hierarchy

```
BaseBlock (base class for config blocks)
    ↓
    Contains:
    - root
    - index files
    - error_pages
    - client_max_body_size
    - autoindex

    ↓ Inherited by
    ├── Container (top-level, holds servers)
    │     Contains: vector<Server>
    │
    ├── Server (one server block)
    │     Contains:
    │     - listen directives (host:port pairs)
    │     - server_name
    │     - vector<LocationConfig>
    │
    └── LocationConfig (one location block)
          Contains:
          - path
          - allowed methods
          - upload_dir
```

### BaseBlock.hpp

**Path:** `src/models/headers/BaseBlock.hpp`

```cpp
class BaseBlock {
protected:
    std::string _root;                           // Document root
    std::pair<u_int16_t, std::string> _returnData; // Redirect info
    size_t _clientMaxBodySize;                   // Max body size
    std::vector<std::string> _indexFiles;        // Default files
    std::map<u_int16_t, std::string> _errorPages; // Custom error pages
    bool _autoIndex;                             // Directory listing

    BaseBlock();
    virtual ~BaseBlock();

public:
    // Getters
    const std::string& getRoot() const;
    const std::vector<std::string>& getIndexFiles() const;
    const std::string* getErrorPage(const u_int16_t code) const;
    bool getAutoIndex() const;

    // Setters
    void setRoot(const std::string& root);
    void insertIndex(const std::vector<std::string>& routes);
    void insertErrorPage(u_int16_t errorCode, const std::string& errorPage);
    void activateAutoIndex();
};
```

### Server.hpp

**Path:** `src/models/headers/Server.hpp`

```cpp
struct ListenCtx {
    u_int16_t port;      // Port number
    std::string addr;    // IP address
};

class Server : public BaseBlock {
private:
    std::vector<ListenCtx> _listens;           // listen directives
    std::vector<std::string> _serverNames;     // server_name directives
    std::vector<LocationConfig> _locations;    // location blocks

public:
    Server();
    ~Server();

    // Listen management
    void insertListen(u_int16_t port = 80, const std::string& addr = "0.0.0.0");
    const std::vector<ListenCtx>& getListens() const;

    // Server names
    void insertServerNames(const std::string& serverName);
    const std::vector<std::string>& getServerNames() const;

    // Location management
    void addLocation(const LocationConfig& location);
    const LocationConfig* findLocation(const std::string& path) const;
};
```

### LocationConfig.hpp

**Path:** `src/models/headers/LocationConfig.hpp`

```cpp
class LocationConfig : public BaseBlock {
private:
    std::string _path;                      // Location path pattern
    std::vector<std::string> _methods;      // Allowed HTTP methods
    std::string _uploadDir;                 // Upload directory

public:
    LocationConfig();
    LocationConfig(const std::string& path);

    void setPath(const std::string& path);
    void addMethod(const std::string& method);
    void setUploadDir(const std::string& dir);

    const std::string& getPath() const;
    bool isMethodAllowed(const std::string& method) const;
    const std::string& getUploadDir() const;
};
```

### Parser Flow

```
Config File (text)
      ↓
readFile() → std::string
      ↓
lexer() → std::vector<Token>
      ↓
checks() → validate tokens
      ↓
parser() → Container
            ↓
        [Server, Server, ...]
            ↓
        Each has [Location, Location, ...]
```

---

## 🌐 Network Layer - SocketManager

**Path:** `src/models/headers/SocketManager.hpp` & `srcs/SocketManager.cpp`

The heart of the server - handles all network I/O.

### Key Responsibilities

1. **Initialize listening sockets** (bind, listen)
2. **Main event loop** (epoll_wait)
3. **Accept new connections**
4. **Read requests**
5. **Write responses**
6. **Handle timeouts**
7. **Manage connection state**

### Class Structure

```cpp
class SocketManager {
private:
    // Listening sockets (one per host:port)
    std::vector<int> listeningSockets;

    // Per-client state
    std::map<int, std::string> requestBuffers;   // fd → partial request
    std::map<int, time_t> lastActivity;          // fd → last I/O timestamp
    std::map<int, std::string> sendBuffers;      // fd → response data

    // Server configurations
    std::vector<Server> serverList;

    // HTTP components
    HttpParser* httpParser;
    HttpResponse* responseBuilder;

    static const int CLIENT_TIMEOUT = 60;  // seconds

public:
    SocketManager();
    ~SocketManager();

    // Initialization
    void setServers(const std::vector<Server>& servers);
    bool initSockets(const std::vector<ServerSocketInfo>& servers);

    // Main loop
    void handleClients();  // Infinite event loop

    // Event handlers
    void acceptNewClient(int readyServerFd, int epoll_fd);
    void handleRequest(int readyServerFd, int epoll_fd);
    void sendBuffer(int fd, int epfd);
    void handleTimeouts(int epoll_fd);

    // Request processing
    void processFullRequest(int fd, int epfd, const std::string& rawRequest);
    HttpRequest* fillRequest(const std::string& rawRequest, Server& server);

    // Response sending
    void sendHttpResponse(int fd, int epfd, const HttpResponse& res);
    void sendHttpError(int fd, const std::string& status, int epfd);

    // Validation
    bool validateRequest(int fd, int epfd);
    bool isRequestTooLarge(int fd);
    bool isHeaderTooLarge(int fd);
};
```

### Main Event Loop

```cpp
void SocketManager::handleClients() {
    // Create epoll instance
    int epoll_fd = epoll_create1(EPOLL_DEFAULT);

    // Add all listening sockets to epoll
    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = listeningSockets[i];
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listeningSockets[i], &ev);
    }

    // Main loop
    while (true) {
        struct epoll_event events[64];
        int n = epoll_wait(epoll_fd, events, 64, 1000);  // 1 sec timeout

        // Process events
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (isServerSocket(fd)) {
                // New connection
                acceptNewClient(fd, epoll_fd);
            }
            else if (events[i].events & EPOLLIN) {
                // Read from client
                handleRequest(fd, epoll_fd);
            }
            else if (events[i].events & EPOLLOUT) {
                // Write to client
                sendBuffer(fd, epoll_fd);
            }
        }

        // Check for timeouts
        handleTimeouts(epoll_fd);
    }
}
```

### Accepting Connections

```cpp
void SocketManager::acceptNewClient(int server_fd, int epoll_fd) {
    // Accept connection
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) return;

    // Set non-blocking
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    // Add to epoll (monitor for read)
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    // Initialize state
    lastActivity[client_fd] = time(NULL);
}
```

### Reading Requests

```cpp
void SocketManager::handleRequest(int fd, int epoll_fd) {
    char buffer[8192];
    ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

    if (n > 0) {
        // Append to request buffer
        requestBuffers[fd].append(buffer, n);
        lastActivity[fd] = time(NULL);

        // Validate size
        if (!validateRequest(fd, epoll_fd)) {
            return;  // Error sent, connection closed
        }

        // Check if request complete
        std::string& req = requestBuffers[fd];
        size_t headerEnd = req.find("\r\n\r\n");

        if (headerEnd != std::string::npos) {
            // Headers complete, check if body complete
            // ... (check Content-Length or chunked)

            processFullRequest(fd, epoll_fd, req);
            requestBuffers.erase(fd);
        }
    }
    else {
        // Connection closed or error
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        requestBuffers.erase(fd);
        lastActivity.erase(fd);
    }
}
```

---

## 🔍 HTTP Parsing - HttpParser

**Path:** `src/models/headers/HttpParser.hpp` & `srcs/HttpParser.cpp`

Converts raw HTTP text into HttpRequest objects.

### Class Structure

```cpp
class HttpParser {
private:
    std::string lastError;

    bool parseRequestLine(const std::string& line,
                         std::string& method,
                         std::string& path,
                         std::string& version);
    bool parseHeaders(const std::string& headerSection, HttpRequest* request);
    bool parseBody(const std::string& body, HttpRequest* request);

public:
    HttpParser();
    ~HttpParser();

    HttpRequest* parseRequest(const std::string& rawRequest, Server& server);
    void clearError();
};
```

### Parsing Flow

```cpp
HttpRequest* HttpParser::parseRequest(const std::string& rawRequest,
                                      Server& server) {
    // 1. Split into lines
    std::istringstream stream(rawRequest);
    std::string line;

    // 2. Parse request line
    std::getline(stream, line);
    std::string method, path, version;
    if (!parseRequestLine(line, method, path, version)) {
        return NULL;
    }

    // 3. Parse headers
    std::string headerSection;
    while (std::getline(stream, line) && line != "\r") {
        headerSection += line + "\n";
    }

    // 4. Create request context
    RequestContext ctx(server, /* ... */);

    // 5. Create appropriate request object (factory pattern)
    HttpRequest* req = makeRequestByMethod(method, ctx);
    if (!req) return NULL;

    // 6. Fill request data
    req->setMethod(method);
    req->setPath(path);
    req->setVersion(version);

    // 7. Parse headers
    parseHeaders(headerSection, req);

    // 8. Parse body (if present)
    std::string bodyData;
    std::getline(stream, bodyData, '\0');  // Read rest
    if (!bodyData.empty()) {
        req->appendBody(bodyData);
    }

    return req;
}
```

---

## 📥 Request Handling - HttpRequest

**Path:** `src/models/headers/HttpRequest.hpp` & `srcs/HttpRequest.cpp`

Polymorphic request handling using inheritance.

### Class Hierarchy

```cpp
// Abstract base class
class HttpRequest {
protected:
    const RequestContext& _ctx;  // Server config, location, etc.
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> query;

public:
    HttpRequest(const RequestContext& ctx);
    virtual ~HttpRequest();

    // Pure virtual - must be implemented by subclasses
    virtual bool validate(std::string& err) const = 0;
    virtual void handle(HttpResponse& res) = 0;

    // Common methods
    const std::string& getMethod() const;
    const std::string& getPath() const;
    // ... getters/setters ...
};

// Concrete implementations
class GetHeadRequest : public HttpRequest {
public:
    GetHeadRequest(const RequestContext& ctx);
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class PostRequest : public HttpRequest {
public:
    PostRequest(const RequestContext& ctx);
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class DeleteRequest : public HttpRequest {
public:
    DeleteRequest(const RequestContext& ctx);
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};
```

### Factory Pattern

```cpp
// From HttpRequest.cpp
HttpRequest* makeRequestByMethod(const std::string& method,
                                 const RequestContext& ctx) {
    if (method == "GET" || method == "HEAD") {
        return new GetHeadRequest(ctx);
    } else if (method == "POST") {
        return new PostRequest(ctx);
    } else if (method == "DELETE") {
        return new DeleteRequest(ctx);
    }
    return NULL;  // Unsupported method
}
```

### GET Request Handling

```cpp
void GetHeadRequest::handle(HttpResponse& res) {
    // 1. Resolve path (root + requested path)
    std::string fullPath = _ctx.getServer().getRoot() + path;

    // 2. Check if file exists
    struct stat st;
    if (stat(fullPath.c_str(), &st) != 0) {
        res.setError(404, "Not Found");
        return;
    }

    // 3. Check if directory
    if (S_ISDIR(st.st_mode)) {
        // Try index files
        const std::vector<std::string>& indexFiles =
            _ctx.getServer().getIndexFiles();

        for (size_t i = 0; i < indexFiles.size(); ++i) {
            std::string indexPath = fullPath + "/" + indexFiles[i];
            if (access(indexPath.c_str(), R_OK) == 0) {
                fullPath = indexPath;
                break;
            }
        }

        // Or show directory listing if enabled
        if (S_ISDIR(st.st_mode) && _ctx.getServer().getAutoIndex()) {
            generateDirectoryListing(fullPath, res);
            return;
        }
    }

    // 4. Read file
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file) {
        res.setError(403, "Forbidden");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // 5. Build response
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", getMimeType(fullPath));
    res.setHeader("Content-Length", toString(content.size()));

    if (method != "HEAD") {  // HEAD doesn't send body
        res.setBody(content);
    }
}
```

---

## 📤 Response Building - HttpResponse

**Path:** `src/models/headers/HttpResponse.hpp` & `srcs/HttpResponse.cpp`

Builds HTTP response messages.

### Class Structure

```cpp
class HttpResponse {
private:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string version;

public:
    HttpResponse();
    ~HttpResponse();

    void setStatus(int code, const std::string& reason);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& b);
    void setVersion(const std::string& v);

    std::string build() const;  // Generate full HTTP response

    // Error helpers
    void setError(int code, const std::string& reason);
    void setErrorFromContext(int code, const RequestContext& ctx);
};
```

### Building Response

```cpp
std::string HttpResponse::build() const {
    std::ostringstream response;

    // Status line
    response << version << " " << statusCode << " "
             << statusMessage << "\r\n";

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    // Blank line
    response << "\r\n";

    // Body
    response << body;

    return response.str();
}
```

---

## 🛠️ Utility Functions

**Path:** `src/utils.cpp`, `includes/utils.hpp`

Common helper functions used throughout the codebase.

```cpp
// String utilities
std::string toLowerStr(const std::string& str);
std::string toUpperStr(const std::string& str);
std::string trim(const std::string& str);

// Number conversion
std::string toString(int n);
std::string toString(size_t n);

// File utilities
bool fileExists(const std::string& path);
bool isDirectory(const std::string& path);
std::string getMimeType(const std::string& path);

// HTTP utilities
std::string urlDecode(const std::string& str);
std::string urlEncode(const std::string& str);
```

---

## 🔨 Build System - Makefile

**Path:** `Makefile`

### Key Targets

```makefile
NAME = webserv

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Build executable
all: $(NAME)

# Compile
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Clean object files
clean:
	rm -f $(OBJS)

# Clean everything
fclean: clean
	rm -f $(NAME)

# Rebuild
re: fclean all
```

### Building

```bash
# Build
make

# Clean and rebuild
make re

# Clean object files
make clean

# Clean everything
make fclean
```

---

## 🧪 Testing Infrastructure

**Path:** `Tests/`

### Test Scripts

```bash
# Run all tests
./Tests/run_all_tests.sh

# Individual test suites
./Tests/core_tests.sh        # Basic functionality
./Tests/post_tests.sh        # File uploads
./Tests/delete_tests.sh      # DELETE requests
./Tests/error_tests.sh       # Error handling
```

### Manual Testing

```bash
# Test with curl
curl http://localhost:8080/
curl -X POST -d "data=value" http://localhost:8080/upload
curl -X DELETE http://localhost:8080/test.txt

# Test with telnet (raw HTTP)
telnet localhost 8080
GET / HTTP/1.1
Host: localhost

# Test with browser
firefox http://localhost:8080/
```

---

## 🎓 Summary

**Key Source Files:**

| File                | Purpose                        |
| ------------------- | ------------------------------ |
| `main.cpp`          | Entry point, initialization    |
| `SocketManager.cpp` | Network I/O, event loop        |
| `HttpParser.cpp`    | Parse raw HTTP requests        |
| `HttpRequest.cpp`   | Request handling (polymorphic) |
| `HttpResponse.cpp`  | Build HTTP responses           |
| `Server.cpp`        | Server configuration           |
| `parser.cpp`        | Config file parser             |

**Data Flow:**

```
Config File → Parser → Container → [Servers]
                                       ↓
                              SocketManager.initSockets()
                                       ↓
                              epoll event loop
                                       ↓
                        Raw HTTP → HttpParser → HttpRequest
                                       ↓
                              HttpRequest.handle()
                                       ↓
                              HttpResponse.build()
                                       ↓
                              Send to client
```

**Next Steps:**

- Read [7_DEVELOPMENT_GUIDE.md](7_DEVELOPMENT_GUIDE.md) for development workflow
- Start with small changes (add a header, modify an error message)
- Use debugger to trace request flow
- Write tests for your changes

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
