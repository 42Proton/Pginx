# 🏛️ Architecture Guide - Pginx HTTP Server

**Priority Document - Read This First!**

This document explains the high-level architecture of the Pginx web server, how components interact, and the flow of data through the system.

---

## 📋 Table of Contents

1. [High-Level Overview](#high-level-overview)
2. [Core Architecture Principles](#core-architecture-principles)
3. [Component Diagram](#component-diagram)
4. [Class Hierarchy](#class-hierarchy)
5. [Request/Response Flow](#requestresponse-flow)
6. [Key Design Decisions](#key-design-decisions)
7. [Threading Model](#threading-model)
8. [Memory Management](#memory-management)

---

## 🎯 High-Level Overview

Pginx is a **single-threaded, event-driven HTTP server** that uses **non-blocking I/O** and **I/O multiplexing** (epoll on Linux) to handle multiple client connections concurrently.

### The Big Picture

```
┌─────────────┐
│   Client    │
│  (Browser)  │
└──────┬──────┘
       │ HTTP Request
       ▼
┌─────────────────────────────────────────────┐
│         SocketManager (Main Loop)           │
│  ┌────────────────────────────────────┐    │
│  │      epoll_wait()                  │    │
│  │  (Monitors all socket events)      │    │
│  └────────────────────────────────────┘    │
└──────────┬───────────────┬──────────────────┘
           │               │
    Accept │               │ Read/Write
           │               │
  ┌────────▼─────┐    ┌────▼──────────────┐
  │ New Client   │    │  Existing Client  │
  │  Connection  │    │   Request/Resp    │
  └──────────────┘    └───────┬───────────┘
                              │
                    ┌─────────▼──────────┐
                    │    HttpParser      │
                    │  (Parse Request)   │
                    └─────────┬──────────┘
                              │
                    ┌─────────▼───────────┐
                    │   HttpRequest       │
                    │  (Create concrete   │
                    │   GET/POST/DELETE)  │
                    └─────────┬───────────┘
                              │
                    ┌─────────▼───────────┐
                    │  HttpRequest        │
                    │   ::handle()        │
                    │  (Process request)  │
                    └─────────┬───────────┘
                              │
                    ┌─────────▼───────────┐
                    │   HttpResponse      │
                    │  (Build response)   │
                    └─────────┬───────────┘
                              │
                              ▼
                       Send to Client
```

---

## ⚡ Core Architecture Principles

### 1. **Single-Threaded Event Loop**

- **No threads, no fork** (except for CGI execution)
- All I/O is **non-blocking**
- One **epoll()** handles all file descriptors

### 2. **Event-Driven Design**

- The server **reacts to events** (readable/writable sockets)
- No polling or busy-waiting
- Efficient CPU usage

### 3. **Non-Blocking I/O**

- All sockets are set to `O_NONBLOCK`
- Never block on `read()`, `write()`, or `accept()`
- Use `epoll_wait()` to know when I/O is ready

### 4. **Separation of Concerns**

- **SocketManager**: Handles all network I/O and event loop
- **HttpParser**: Parses raw HTTP requests
- **HttpRequest**: Represents and processes requests (polymorphic)
- **HttpResponse**: Builds HTTP responses
- **Server/Container**: Configuration and routing

### 5. **Polymorphic Request Handling**

- Base class `HttpRequest` defines the interface
- Subclasses (`GetHeadRequest`, `PostRequest`, `DeleteRequest`) implement specific logic
- Factory pattern creates the appropriate request type

---

## 🗺️ Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                          main.cpp                               │
│  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐       │
│  │   Parser     │──▶│  Container   │──▶│SocketManager │       │
│  │ (Config)     │   │  (Servers)   │   │  (Network)   │       │
│  └──────────────┘   └──────────────┘   └──────────────┘       │
└─────────────────────────────────────────────────────────────────┘
                                              │
              ┌───────────────────────────────┼───────────────────────────┐
              │                               │                           │
    ┌─────────▼────────┐          ┌──────────▼────────┐     ┌───────────▼────────┐
    │   HttpParser     │          │   HttpRequest     │     │   HttpResponse     │
    │ ┌──────────────┐ │          │  (Abstract Base)  │     │ ┌────────────────┐ │
    │ │parseRequest()│ │          │                   │     │ │setStatus()     │ │
    │ │parseHeaders()│ │          │  ┌──────────────┐ │     │ │setHeader()     │ │
    │ │parseBody()   │ │          │  │validate()    │ │     │ │setBody()       │ │
    │ └──────────────┘ │          │  │handle()      │ │     │ │build()         │ │
    └──────────────────┘          │  └──────────────┘ │     │ └────────────────┘ │
                                  │                   │     └────────────────────┘
                                  │  ┌──────────────┐ │
                                  │  │Subclasses:   │ │
                                  │  │-GetHead      │ │
                                  │  │-Post         │ │
                                  │  │-Delete       │ │
                                  │  └──────────────┘ │
                                  └───────────────────┘
```

### Component Responsibilities

| Component          | Responsibility                                    |
| ------------------ | ------------------------------------------------- |
| **main.cpp**       | Entry point, initializes everything               |
| **Parser**         | Reads and parses configuration file               |
| **Container**      | Holds all Server configurations                   |
| **Server**         | Represents one `server { }` block in config       |
| **LocationConfig** | Represents one `location { }` block               |
| **SocketManager**  | Main event loop, accepts connections, handles I/O |
| **HttpParser**     | Converts raw bytes into HttpRequest object        |
| **HttpRequest**    | Abstract base class for all request types         |
| **GetHeadRequest** | Handles GET and HEAD methods                      |
| **PostRequest**    | Handles POST (file uploads)                       |
| **DeleteRequest**  | Handles DELETE method                             |
| **HttpResponse**   | Builds HTTP response messages                     |

---

## 🌳 Class Hierarchy

### Configuration Classes

```
BaseBlock (abstract base for config blocks)
    ├── Container (holds multiple servers)
    │     └── std::vector<Server>
    ├── Server (one server block)
    │     └── std::vector<LocationConfig>
    └── LocationConfig (one location block)
```

**BaseBlock** provides common configuration directives:

- `root` - Document root directory
- `index` - Default index files
- `error_pages` - Custom error pages
- `client_max_body_size` - Max request body size
- `autoindex` - Directory listing enabled/disabled

**Server** adds:

- `listen` directives (host:port pairs)
- `server_name` directives
- Multiple `LocationConfig` objects

**LocationConfig** adds:

- `path` - URL path pattern
- `methods` - Allowed HTTP methods
- `upload_dir` - Where to store uploaded files

### Request/Response Classes

```
HttpRequest (abstract base)
    ├── GetHeadRequest (GET and HEAD methods)
    ├── PostRequest (POST with file uploads)
    ├── DeleteRequest (DELETE files)
    ├── PutRequest (Not implemented yet)
    └── PatchRequest (Not implemented yet)

HttpResponse (builds HTTP responses)

HttpParser (parses raw HTTP into HttpRequest)
```

### Network Classes

```
SocketManager
    ├── std::vector<int> listeningSockets
    ├── std::map<int, std::string> requestBuffers
    ├── std::map<int, std::string> sendBuffers
    ├── std::map<int, time_t> lastActivity
    └── std::vector<Server> serverList
```

---

## 🔄 Request/Response Flow

Let's trace what happens when a client sends `GET /index.html HTTP/1.1`:

### Step 1: Connection Establishment

```cpp
// In SocketManager::handleClients()
int epoll_fd = epoll_create1(EPOLL_DEFAULT);

// Add all listening sockets to epoll
for each listening_socket:
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listening_socket, EPOLLIN);

// Main event loop
while (true) {
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);
    for (int i = 0; i < n; i++) {
        if (isServerSocket(events[i].data.fd)) {
            acceptNewClient(events[i].data.fd, epoll_fd);
        } else if (events[i].events & EPOLLIN) {
            handleRequest(events[i].data.fd, epoll_fd);
        } else if (events[i].events & EPOLLOUT) {
            sendBuffer(events[i].data.fd, epoll_fd);
        }
    }
}
```

### Step 2: Accept New Client

```cpp
// In SocketManager::acceptNewClient()
int client_fd = accept(server_fd, ...);
fcntl(client_fd, F_SETFL, O_NONBLOCK);  // Make non-blocking
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, EPOLLIN);
lastActivity[client_fd] = time(NULL);  // Track timeout
```

### Step 3: Read Request Data

```cpp
// In SocketManager::handleRequest()
char buffer[8192];
ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);

if (n > 0) {
    requestBuffers[client_fd].append(buffer, n);

    // Check if we have a complete request
    if (requestComplete(requestBuffers[client_fd])) {
        processFullRequest(client_fd, epoll_fd, requestBuffers[client_fd]);
    }
}
```

### Step 4: Parse Request

```cpp
// In SocketManager::processFullRequest()
HttpRequest* req = httpParser->parseRequest(rawRequest, server);

// Inside HttpParser::parseRequest()
1. Extract request line: "GET /index.html HTTP/1.1"
2. Parse method, path, version
3. Parse headers (Host, Content-Length, etc.)
4. Parse body (if present)
5. Create appropriate HttpRequest subclass (GetHeadRequest)
6. Return the request object
```

### Step 5: Validate and Handle Request

```cpp
// Back in processFullRequest()
std::string err;
if (!req->validate(err)) {
    // Send error response
    HttpResponse errorRes;
    errorRes.setError(400, "Bad Request");
    sendHttpResponse(client_fd, epoll_fd, errorRes);
    return;
}

// Process the request
HttpResponse res;
req->handle(res);  // Polymorphic call

// In GetHeadRequest::handle()
1. Resolve the file path (root + path)
2. Check if file exists
3. Check permissions
4. Read file content
5. Set appropriate headers (Content-Type, Content-Length)
6. Fill response body with file content
```

### Step 6: Build and Send Response

```cpp
// In SocketManager::sendHttpResponse()
std::string responseText = res.build();

// HttpResponse::build() creates:
// HTTP/1.1 200 OK
// Content-Type: text/html
// Content-Length: 1234
//
// <html>...</html>

sendBuffers[client_fd] = responseText;

// Modify epoll to monitor for EPOLLOUT (writable)
epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, EPOLLOUT);
```

### Step 7: Write Response

```cpp
// In SocketManager::sendBuffer()
ssize_t sent = send(client_fd, sendBuffers[client_fd].data(),
                    sendBuffers[client_fd].size(), 0);

if (sent > 0) {
    sendBuffers[client_fd].erase(0, sent);
}

if (sendBuffers[client_fd].empty()) {
    // All data sent, clean up
    close(client_fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
}
```

### Complete Flow Diagram

```
Client          SocketManager       HttpParser      HttpRequest      HttpResponse
  |                  |                   |                |                |
  |--- TCP SYN ----->|                   |                |                |
  |<-- SYN-ACK ------|                   |                |                |
  |--- ACK --------->|                   |                |                |
  |                  |                   |                |                |
  | epoll_wait()     |                   |                |                |
  | returns EPOLLIN  |                   |                |                |
  |                  |                   |                |                |
  |--- GET / ------->|                   |                |                |
  |                  |                   |                |                |
  |                  |--- parseRequest ->|                |                |
  |                  |                   |--- new Get --->|                |
  |                  |                   |                |                |
  |                  |<-- HttpRequest ---|                |                |
  |                  |                   |                |                |
  |                  |--- validate() ------------------->|                |
  |                  |<-- true --------------------------|                |
  |                  |                   |                |                |
  |                  |--- handle(res) ------------------>|                |
  |                  |                   |                |--- setStatus ->|
  |                  |                   |                |--- setBody --->|
  |                  |<-- (res filled) ------------------|                |
  |                  |                   |                |                |
  |                  |--- build() ------------------------------>|        |
  |                  |<-- "HTTP/1.1..." <------------------------|        |
  |                  |                   |                |                |
  |<-- HTTP/1.1 -----|                   |                |                |
  |    200 OK        |                   |                |                |
  |    <html>...     |                   |                |                |
  |                  |                   |                |                |
```

---

## 🎯 Key Design Decisions

### 1. **Why Single-Threaded?**

**Advantages:**

- Simpler to reason about (no race conditions)
- No need for mutexes/locks
- Easier to debug
- Lower memory overhead
- Sufficient for I/O-bound workload

**Trade-offs:**

- Can't utilize multiple CPU cores
- CPU-intensive CGI blocks the event loop (solved by fork)

### 2. **Why epoll?**

**Advantages over select():**

- O(1) performance regardless of number of FDs
- No hard limit on number of file descriptors
- More efficient for large numbers of connections

**Note:** The code is designed to support poll/select as alternatives.

### 3. **Why Polymorphism for Request Types?**

```cpp
// Instead of:
void handleRequest(HttpRequest& req) {
    if (req.method == "GET") {
        // ... 100 lines of GET logic
    } else if (req.method == "POST") {
        // ... 150 lines of POST logic
    } // ...
}

// We have:
req->handle(res);  // Each subclass implements its own logic
```

**Benefits:**

- Cleaner code organization
- Easier to add new methods
- Each class has single responsibility
- Better testability

### 4. **Why Configuration Inheritance (BaseBlock)?**

```cpp
class BaseBlock {
    // Common directives: root, index, error_pages, etc.
};

class Server : public BaseBlock { /* ... */ };
class LocationConfig : public BaseBlock { /* ... */ };
```

**Benefits:**

- DRY (Don't Repeat Yourself)
- Locations can override server defaults
- Consistent interface

### 5. **Why Buffering?**

```cpp
std::map<int, std::string> requestBuffers;   // Incoming data
std::map<int, std::string> sendBuffers;      // Outgoing data
```

**Reasons:**

- Non-blocking I/O might not read/write everything at once
- HTTP requests might arrive in multiple packets
- Large responses need to be sent in chunks

---

## 🔀 Threading Model

### Current: Single-Threaded Event Loop

```
┌─────────────────────────────────────┐
│     Main Thread (Event Loop)        │
│  ┌────────────────────────────┐    │
│  │   while (true) {           │    │
│  │     epoll_wait()           │    │
│  │     handle events          │    │
│  │   }                        │    │
│  └────────────────────────────┘    │
└─────────────────────────────────────┘
```

**Exception: CGI Execution**

```
Main Thread                    CGI Process
    |                               |
    |--- fork() ------------------>|
    |                          exec("php-cgi")
    |--- waitpid() (non-block)     |
    |                          (running PHP)
    |<-- SIGCHLD ------------------|
    |                               |
    |--- read output ------------->|
```

---

## 💾 Memory Management

### Resource Acquisition Is Initialization (RAII)

```cpp
class SocketManager {
    HttpParser* httpParser;
    HttpResponse* responseBuilder;

public:
    SocketManager()
        : httpParser(new HttpParser()),
          responseBuilder(new HttpResponse()) {}

    ~SocketManager() {
        delete httpParser;
        delete responseBuilder;
    }
};
```

**Principles:**

1. **Resources allocated in constructors**
2. **Resources freed in destructors**
3. **No manual memory management in most code**
4. **STL containers manage their own memory**

### Memory Leaks Prevention

```cpp
// ✅ Good: Using STL containers (automatic cleanup)
std::vector<Server> servers;
std::map<int, std::string> buffers;

// ✅ Good: RAII with destructors
HttpRequest* req = makeRequestByMethod(method, ctx);
// ... use req ...
delete req;  // Clean up when done

// ❌ Bad: Forgetting to delete
HttpRequest* req = new GetHeadRequest(ctx);
return;  // Memory leak!
```

### File Descriptor Management

```cpp
// Always close FDs when done
void SocketManager::closeSocket() {
    for (size_t i = 0; i < listeningSockets.size(); ++i) {
        close(listeningSockets[i]);
    }
    listeningSockets.clear();
}
```

---

## 🔍 Key Data Structures

### 1. Request Buffers

```cpp
std::map<int, std::string> requestBuffers;
// Key: client file descriptor
// Value: accumulated request data
```

**Purpose:** Store incomplete HTTP requests until they're fully received.

### 2. Send Buffers

```cpp
std::map<int, std::string> sendBuffers;
// Key: client file descriptor
// Value: response data to be sent
```

**Purpose:** Store response data when socket isn't ready to accept all data at once.

### 3. Last Activity Tracker

```cpp
std::map<int, time_t> lastActivity;
// Key: client file descriptor
// Value: timestamp of last I/O
```

**Purpose:** Implement timeout mechanism to close idle connections.

### 4. Server List

```cpp
std::vector<Server> serverList;
```

**Purpose:** Hold all configured virtual hosts for request routing.

---

## 🚦 Error Handling Strategy

### 1. **Network Errors**

```cpp
ssize_t n = recv(fd, buffer, size, 0);
if (n <= 0) {
    if (n == 0) {
        // Client closed connection gracefully
        close(fd);
    } else {
        // Error occurred (but we don't check errno!)
        close(fd);
    }
}
```

### 2. **HTTP Errors**

```cpp
if (!request->validate(err)) {
    HttpResponse errorRes;
    errorRes.setError(400, "Bad Request");
    sendHttpResponse(fd, epoll_fd, errorRes);
}
```

### 3. **File Errors**

```cpp
if (!fileExists(path)) {
    res.setError(404, "Not Found");
} else if (!fileReadable(path)) {
    res.setError(403, "Forbidden");
}
```

---

## 📊 Performance Characteristics

| Aspect | Implementation | Performance |
| --- | --- | --- |
| **Accept** | Non-blocking with epoll | O(1) per connection |
| **Read/Write** | Non-blocking with buffering | O(1) per event |
| **Event Loop** | epoll_wait | O(N) where N = active events |
| **Request Routing** | Linear search through locations | O(M) where M = number of locations |
| **Timeout Checking** | Iterate all connections | O(C) where C = number of connections |

**Bottlenecks to watch:**

- Too many concurrent connections (increase `ulimit -n`)
- Large file serving (consider sendfile() optimization)
- CGI execution (each fork is expensive)

---

## 🎓 Summary

**Key Takeaways:**

1. **Event-Driven Architecture** - Single epoll loop handles everything
2. **Non-Blocking I/O** - Never block on socket operations
3. **Polymorphic Requests** - Clean separation of HTTP method logic
4. **Configuration Hierarchy** - BaseBlock → Server → LocationConfig
5. **Buffered I/O** - Handle partial reads/writes gracefully
6. **RAII Memory Management** - Destructors clean up resources
7. **No errno checking** - Design doesn't rely on errno after I/O

**Next Steps:**

- Read [3_CPP_FOR_C_DEVELOPERS.md](3_CPP_FOR_C_DEVELOPERS.md) if you need C++ refresher
- Read [4_NETWORK_PROGRAMMING.md](4_NETWORK_PROGRAMMING.md) to understand sockets/epoll
- Read [6_CODEBASE_GUIDE.md](6_CODEBASE_GUIDE.md) for detailed code walkthrough

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
