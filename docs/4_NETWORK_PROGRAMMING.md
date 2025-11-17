# 🌐 Network Programming Fundamentals

**Everything You Need to Know About Sockets, TCP/IP, and I/O Multiplexing**

This document explains the networking concepts and APIs used in Pginx from the ground up.

---

## 📋 Table of Contents

1. [Networking Model Overview](#networking-model-overview)
2. [TCP/IP Protocol Stack](#tcpip-protocol-stack)
3. [Sockets API](#sockets-api)
4. [Blocking vs Non-Blocking I/O](#blocking-vs-non-blocking-io)
5. [I/O Multiplexing](#io-multiplexing)
6. [epoll Deep Dive](#epoll-deep-dive)
7. [Connection Lifecycle](#connection-lifecycle)
8. [Error Handling in Network Code](#error-handling-in-network-code)
9. [Common Pitfalls](#common-pitfalls)
10. [Performance Considerations](#performance-considerations)

---

## 🎯 Networking Model Overview

### OSI Model (Simplified)

```
┌─────────────────────────────────────────┐
│  Application Layer (HTTP, FTP, SMTP)   │  ← We work here
├─────────────────────────────────────────┤
│  Transport Layer (TCP, UDP)            │  ← Sockets API interface
├─────────────────────────────────────────┤
│  Network Layer (IP)                    │  ← Kernel handles this
├─────────────────────────────────────────┤
│  Link Layer (Ethernet, WiFi)           │  ← Hardware/drivers
├─────────────────────────────────────────┤
│  Physical Layer (Cables, Radio)        │  ← Physical medium
└─────────────────────────────────────────┘
```

**For web servers:**

- **Application Layer**: HTTP protocol (we implement this)
- **Transport Layer**: TCP connections (Sockets API)
- **Everything below**: Managed by OS kernel

---

## 🔗 TCP/IP Protocol Stack

### What is TCP?

**TCP (Transmission Control Protocol)** provides:

- ✅ **Reliable** delivery (no lost packets)
- ✅ **Ordered** delivery (packets arrive in order)
- ✅ **Connection-oriented** (must establish connection first)
- ✅ **Flow control** (doesn't overwhelm receiver)
- ✅ **Error detection** (checksums)

### TCP Three-Way Handshake

```
Client                          Server
  |                                |
  |--- SYN (Synchronize) --------->|  "I want to connect"
  |                                |
  |<-- SYN-ACK (Acknowledge) ------|  "OK, let's connect"
  |                                |
  |--- ACK (Acknowledge) --------->|  "Connection established"
  |                                |
  |<===== Data Transfer =========>|
  |                                |
```

### TCP Connection Termination

```
Client                          Server
  |                                |
  |--- FIN (Finish) -------------->|  "I'm done sending"
  |                                |
  |<-- ACK ---------------------|  "OK"
  |                                |
  |<-- FIN ------------------------|  "I'm done too"
  |                                |
  |--- ACK ----------------------->|  "Goodbye"
  |                                |
```

### IP Addresses and Ports

```
┌──────────────────────────────┐
│  IP Address: 192.168.1.100   │  ← Identifies host (like street address)
│  Port: 8080                  │  ← Identifies application (like apartment number)
└──────────────────────────────┘

Special addresses:
- 0.0.0.0       : All interfaces (for servers)
- 127.0.0.1     : Localhost (loopback)
- 192.168.x.x   : Private network
```

### Socket = IP + Port + Protocol

```
Socket Endpoint = (IP Address, Port Number, Protocol)

Example:
  (192.168.1.100, 8080, TCP)
  (127.0.0.1, 80, TCP)
```

---

## 🔌 Sockets API

Sockets are the programming interface to the TCP/IP stack.

### Socket Lifecycle (Server)

```
   socket()        Create a socket endpoint
      ↓
   bind()          Associate socket with address:port
      ↓
   listen()        Mark socket as passive (ready to accept)
      ↓
   accept()        Wait for client connection (blocks or non-blocking)
      ↓
   read()/write()  Communicate with client
      ↓
   close()         Close connection
```

### 1. socket() - Create Socket

```cpp
#include <sys/socket.h>

int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
//                      ↑        ↑           ↑
//                      |        |           └─ Protocol (0 = auto)
//                      |        └─ Socket type (STREAM = TCP)
//                      └─ Address family (INET = IPv4)

if (listen_fd == -1) {
    perror("socket");
    // Handle error
}
```

**Returns:** File descriptor (just an integer!)

### 2. bind() - Bind Socket to Address

```cpp
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in addr;
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;                    // IPv4
addr.sin_port = htons(8080);                  // Port 8080 (network byte order!)
addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // Listen on all interfaces

if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    // Handle error (maybe port already in use)
}
```

**Important:** `htons()` converts host byte order to network byte order (big-endian).

### 3. listen() - Mark as Passive Socket

```cpp
if (listen(listen_fd, 10) == -1) {
    //                  ↑
    //                  └─ Backlog: max queued connections
    perror("listen");
    // Handle error
}
```

Now the socket is ready to accept incoming connections!

### 4. accept() - Accept Client Connection

```cpp
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);

int client_fd = accept(listen_fd,
                       (struct sockaddr*)&client_addr,
                       &client_len);

if (client_fd == -1) {
    perror("accept");
    // Handle error
}

// Now client_fd can be used to communicate with this client
```

**accept() returns a NEW file descriptor** for the client connection.

### 5. read()/recv() - Receive Data

```cpp
char buffer[4096];
ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
//                  ↑          ↑         ↑           ↑
//                  |          |         |           └─ Flags (0 = normal)
//                  |          |         └─ Max bytes to read
//                  |          └─ Where to store data
//                  └─ Socket file descriptor

if (n > 0) {
    // Received n bytes
    buffer[n] = '\0';  // Null-terminate if treating as string
    // Process data
} else if (n == 0) {
    // Client closed connection
    close(client_fd);
} else {
    // Error occurred
    perror("recv");
}
```

### 6. write()/send() - Send Data

```cpp
const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
ssize_t sent = send(client_fd, response, strlen(response), 0);

if (sent == -1) {
    perror("send");
} else if (sent < (ssize_t)strlen(response)) {
    // Partial send (common in non-blocking mode!)
    // Need to send remaining data later
}
```

### 7. close() - Close Socket

```cpp
close(client_fd);     // Close client connection
close(listen_fd);     // Close listening socket
```

### From Our Codebase

```cpp
// In SocketManager.cpp - initSockets()
int listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

int opt = 1;
setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//                                 ↑
//                                 └─ Allow reusing address immediately

bind(listen_fd, p->ai_addr, p->ai_addrlen);
listen(listen_fd, 10);

// Store listening socket
listeningSockets.push_back(listen_fd);
```

---

## ⏱️ Blocking vs Non-Blocking I/O

### Blocking I/O (Default)

```cpp
// Blocking accept - waits forever until client connects
int client_fd = accept(listen_fd, NULL, NULL);  // ← Blocks here!

// Blocking read - waits until data arrives
ssize_t n = recv(client_fd, buffer, size, 0);   // ← Blocks here!
```

**Problem:** Server can only handle ONE client at a time!

```
Client 1: Connected
Client 2: Waiting... (server stuck in recv())
Client 3: Waiting... (server stuck in recv())
```

### Non-Blocking I/O

```cpp
#include <fcntl.h>

// Set socket to non-blocking mode
int flags = fcntl(client_fd, F_GETFL, 0);
fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

// Now operations return immediately
ssize_t n = recv(client_fd, buffer, size, 0);

if (n > 0) {
    // Received data
} else if (n == 0) {
    // Connection closed
} else {  // n == -1
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available RIGHT NOW (not an error!)
        // Try again later
    } else {
        // Real error
        perror("recv");
    }
}
```

**But how do we know when to try again?** → **I/O Multiplexing!**

---

## 🔀 I/O Multiplexing

**Problem:** How to handle multiple clients without threads?

**Solution:** Monitor multiple file descriptors and only operate on ready ones.

### Three Options

| Method       | Complexity | Performance | Linux Support   |
| ------------ | ---------- | ----------- | --------------- |
| **select()** | Simple     | O(n)        | ✅              |
| **poll()**   | Moderate   | O(n)        | ✅              |
| **epoll()**  | Complex    | O(1)        | ✅ (Linux only) |

### select() - Oldest Method

```cpp
fd_set read_fds;
FD_ZERO(&read_fds);
FD_SET(listen_fd, &read_fds);      // Monitor listening socket
FD_SET(client_fd1, &read_fds);     // Monitor client 1
FD_SET(client_fd2, &read_fds);     // Monitor client 2

struct timeval timeout;
timeout.tv_sec = 5;
timeout.tv_usec = 0;

int ready = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
//                     ↑           ↑        ↑     ↑       ↑
//                     |           |        |     |       └─ Timeout
//                     |           |        |     └─ Exception FDs
//                     |           |        └─ Write FDs
//                     |           └─ Read FDs
//                     └─ Highest FD + 1

if (ready > 0) {
    if (FD_ISSET(listen_fd, &read_fds)) {
        // New client connection
        accept(...);
    }
    if (FD_ISSET(client_fd1, &read_fds)) {
        // Client 1 sent data
        recv(...);
    }
    // Check all FDs...
}
```

**Limitations:**

- Maximum 1024 file descriptors (FD_SETSIZE)
- O(n) performance (must check all FDs)
- FD sets must be rebuilt every call

### poll() - Improvement over select()

```cpp
struct pollfd fds[100];

fds[0].fd = listen_fd;
fds[0].events = POLLIN;  // Monitor for read events

fds[1].fd = client_fd1;
fds[1].events = POLLIN;

fds[2].fd = client_fd2;
fds[2].events = POLLIN;

int ready = poll(fds, 3, 5000);  // timeout in milliseconds
//                ↑   ↑    ↑
//                |   |    └─ Timeout
//                |   └─ Number of FDs
//                └─ Array of FDs

if (ready > 0) {
    for (int i = 0; i < 3; i++) {
        if (fds[i].revents & POLLIN) {
            // fds[i].fd is ready for reading
        }
    }
}
```

**Better than select() but still O(n).**

---

## 🚀 epoll Deep Dive

**epoll is the modern, efficient way on Linux!**

### epoll API

| Function            | Purpose                               |
| ------------------- | ------------------------------------- |
| **epoll_create1()** | Create epoll instance                 |
| **epoll_ctl()**     | Add/modify/remove FDs from monitoring |
| **epoll_wait()**    | Wait for events on monitored FDs      |

### Step 1: Create epoll Instance

```cpp
#include <sys/epoll.h>

int epoll_fd = epoll_create1(0);  // Flags (0 = default)

if (epoll_fd == -1) {
    perror("epoll_create1");
    // Handle error
}
```

**Returns:** File descriptor for the epoll instance (yes, epoll uses FD too!)

### Step 2: Add FDs to Monitor

```cpp
struct epoll_event ev;
ev.events = EPOLLIN;              // Monitor for read events
//          ↑
//          Options: EPOLLIN, EPOLLOUT, EPOLLERR, EPOLLHUP, EPOLLET

ev.data.fd = listen_fd;           // Store the FD we're monitoring

int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
//                      ↑           ↑             ↑        ↑
//                      |           |             |        └─ Event settings
//                      |           |             └─ FD to add
//                      |           └─ Operation (ADD/MOD/DEL)
//                      └─ Epoll instance

if (ret == -1) {
    perror("epoll_ctl");
}
```

**Operations:**

- `EPOLL_CTL_ADD` - Add FD to monitoring
- `EPOLL_CTL_MOD` - Modify events for FD
- `EPOLL_CTL_DEL` - Remove FD from monitoring

**Events:**

- `EPOLLIN` - Data available for reading
- `EPOLLOUT` - Ready for writing
- `EPOLLERR` - Error condition
- `EPOLLHUP` - Hang up (connection closed)
- `EPOLLET` - Edge-triggered mode (advanced)

### Step 3: Wait for Events

```cpp
#define MAX_EVENTS 64

struct epoll_event events[MAX_EVENTS];

int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//                         ↑         ↑         ↑        ↑
//                         |         |         |        └─ Timeout (-1 = infinite)
//                         |         |         └─ Max events to return
//                         |         └─ Array to store results
//                         └─ Epoll instance

if (ready == -1) {
    perror("epoll_wait");
} else if (ready == 0) {
    // Timeout (won't happen with -1)
} else {
    // Process ready events
    for (int i = 0; i < ready; i++) {
        int fd = events[i].data.fd;

        if (events[i].events & EPOLLIN) {
            // fd is ready for reading
            handleRead(fd);
        }
        if (events[i].events & EPOLLOUT) {
            // fd is ready for writing
            handleWrite(fd);
        }
        if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
            // Error or hangup
            close(fd);
        }
    }
}
```

### From Our Codebase

```cpp
// In SocketManager::handleClients()
int epoll_fd = epoll_create1(EPOLL_DEFAULT);

// Add all listening sockets
for (size_t i = 0; i < listeningSockets.size(); ++i) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listeningSockets[i];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listeningSockets[i], &ev);
}

// Main event loop
while (true) {
    struct epoll_event events[64];
    int n = epoll_wait(epoll_fd, events, 64, 1000);  // 1 second timeout

    for (int i = 0; i < n; i++) {
        int fd = events[i].data.fd;

        if (isServerSocket(fd)) {
            // New connection
            acceptNewClient(fd, epoll_fd);
        } else if (events[i].events & EPOLLIN) {
            // Existing client sent data
            handleRequest(fd, epoll_fd);
        } else if (events[i].events & EPOLLOUT) {
            // Client ready to receive response
            sendBuffer(fd, epoll_fd);
        }
    }

    // Handle timeouts
    handleTimeouts(epoll_fd);
}
```

### Why epoll is Fast

```
select/poll:
  Kernel must iterate ALL monitored FDs each call: O(n)

epoll:
  Kernel maintains ready list, only returns READY FDs: O(1)
```

**Example with 10,000 connections:**

- **select/poll:** Check all 10,000 FDs
- **epoll:** Only return (e.g.) 5 FDs that are actually ready

---

## 🔄 Connection Lifecycle

### Complete Server Flow

```cpp
// 1. Create and configure listening socket
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
int opt = 1;
setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
addr.sin_addr.s_addr = INADDR_ANY;

bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
listen(listen_fd, 10);

// 2. Set non-blocking
fcntl(listen_fd, F_SETFL, O_NONBLOCK);

// 3. Create epoll and add listening socket
int epoll_fd = epoll_create1(0);
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = listen_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

// 4. Main event loop
while (true) {
    struct epoll_event events[64];
    int n = epoll_wait(epoll_fd, events, 64, -1);

    for (int i = 0; i < n; i++) {
        int fd = events[i].data.fd;

        if (fd == listen_fd) {
            // 5. Accept new client
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd == -1) continue;

            // Set non-blocking
            fcntl(client_fd, F_SETFL, O_NONBLOCK);

            // Add to epoll
            struct epoll_event client_ev;
            client_ev.events = EPOLLIN;
            client_ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);

        } else if (events[i].events & EPOLLIN) {
            // 6. Read from client
            char buffer[4096];
            ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

            if (n > 0) {
                // Process data
                // When response ready, switch to EPOLLOUT
                struct epoll_event mod_ev;
                mod_ev.events = EPOLLOUT;
                mod_ev.data.fd = fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &mod_ev);

            } else if (n == 0) {
                // 7. Client closed connection
                close(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            }

        } else if (events[i].events & EPOLLOUT) {
            // 8. Send response
            const char* response = "HTTP/1.1 200 OK\r\n\r\nHello";
            ssize_t sent = send(fd, response, strlen(response), 0);

            if (sent >= (ssize_t)strlen(response)) {
                // All data sent
                close(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            }
        }
    }
}
```

---

## ⚠️ Error Handling in Network Code

### Common Error Codes

```cpp
#include <errno.h>

if (n == -1) {
    switch (errno) {
        case EAGAIN:      // or EWOULDBLOCK
            // No data available (non-blocking socket)
            // This is NOT an error! Try again later
            break;

        case EINTR:
            // System call interrupted by signal
            // Retry the operation
            break;

        case ECONNRESET:
            // Connection reset by peer
            close(fd);
            break;

        case EPIPE:
            // Broken pipe (client closed connection)
            close(fd);
            break;

        default:
            perror("recv");
            close(fd);
    }
}
```

### **Important for Pginx:** Don't Check errno After I/O!

From the subject requirements:

> ⚠️ **Checking the value of errno to adjust the server behaviour is strictly forbidden after performing a read or write operation.**

**Why?** The project wants you to design around epoll events, not error codes.

```cpp
// ❌ Forbidden in this project
ssize_t n = recv(fd, buffer, size, 0);
if (n == -1 && errno == EAGAIN) {  // Don't do this!
    // ...
}

// ✅ Correct approach
ssize_t n = recv(fd, buffer, size, 0);
if (n <= 0) {
    // Close connection or handle error
    // Don't look at errno!
    close(fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}
```

---

## 🚨 Common Pitfalls

### 1. Forgetting to Set Non-Blocking

```cpp
// ❌ Bad
int client_fd = accept(listen_fd, NULL, NULL);
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
// Socket is still blocking!

// ✅ Good
int client_fd = accept(listen_fd, NULL, NULL);
fcntl(client_fd, F_SETFL, O_NONBLOCK);  // Set non-blocking!
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
```

### 2. Partial Reads/Writes

```cpp
// ❌ Bad: Assumes all data received at once
char buffer[4096];
ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
// HTTP request might arrive in multiple packets!

// ✅ Good: Accumulate data
std::string& requestBuffer = requestBuffers[fd];
char buffer[4096];
ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
if (n > 0) {
    requestBuffer.append(buffer, n);

    // Check if complete
    if (requestComplete(requestBuffer)) {
        processRequest(fd, requestBuffer);
    }
}
```

### 3. Forgetting to Remove FD from epoll

```cpp
// ❌ Bad
close(client_fd);
// epoll still monitoring it!

// ✅ Good
epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
close(client_fd);
```

### 4. Not Handling EAGAIN Correctly

```cpp
// ❌ Bad: Treats EAGAIN as error
ssize_t n = send(fd, data, size, 0);
if (n == -1) {
    close(fd);  // Wrong! Might be EAGAIN
}

// ✅ Good: Store remaining data for later
ssize_t n = send(fd, data, size, 0);
if (n > 0) {
    if (n < size) {
        // Partial send, store remaining
        sendBuffers[fd].append(data + n, size - n);

        // Switch to monitoring EPOLLOUT
        struct epoll_event ev;
        ev.events = EPOLLOUT;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    }
} else if (n == 0 || n == -1) {
    // Connection closed or error
    close(fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}
```

### 5. Blocking on Disk I/O

```cpp
// ⚠️ Careful: File read() can block!
// From subject: "You are not required to use poll() for regular disk files"

int file_fd = open("large_file.txt", O_RDONLY);
// Don't add file_fd to epoll!

char buffer[4096];
ssize_t n = read(file_fd, buffer, sizeof(buffer));  // May block
close(file_fd);

// This is OK because disk files are exempt from non-blocking requirement
```

---

## 📊 Performance Considerations

### 1. Connection Limits

```bash
# Check current limit
ulimit -n

# Increase limit (temporary)
ulimit -n 10000

# Increase limit (permanent) - edit /etc/security/limits.conf
* soft nofile 10000
* hard nofile 10000
```

### 2. TCP Tuning

```cpp
// Disable Nagle's algorithm (for low-latency)
int flag = 1;
setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

// Increase socket buffer sizes
int bufsize = 65536;
setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));

// Enable keep-alive
int keepalive = 1;
setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
```

### 3. Timeout Management

```cpp
// Track last activity per connection
std::map<int, time_t> lastActivity;

// Update on I/O
lastActivity[fd] = time(NULL);

// Periodically check for timeouts
void handleTimeouts() {
    time_t now = time(NULL);

    for (std::map<int, time_t>::iterator it = lastActivity.begin();
         it != lastActivity.end(); ) {

        if (now - it->second > TIMEOUT_SECONDS) {
            int fd = it->first;
            close(fd);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

            // Erase from map (careful with iterator!)
            lastActivity.erase(it++);
        } else {
            ++it;
        }
    }
}
```

---

## 🎓 Summary

**Key Networking Concepts:**

1. **TCP/IP** - Reliable, ordered, connection-oriented protocol
2. **Sockets** - Programming interface to network (just file descriptors!)
3. **Non-blocking I/O** - Operations return immediately, use EAGAIN
4. **epoll** - Efficient I/O multiplexing (O(1) performance)
5. **Event Loop** - Single thread handles multiple connections
6. **Buffering** - Handle partial reads/writes

**Critical APIs:**

- `socket()` → `bind()` → `listen()` → `accept()`
- `recv()` / `send()`
- `fcntl()` with `O_NONBLOCK`
- `epoll_create1()` → `epoll_ctl()` → `epoll_wait()`

**Remember:**

- Always set sockets to non-blocking mode
- Always remove FDs from epoll before closing
- Handle partial reads/writes with buffering
- Don't check errno after I/O operations (project rule!)
- Regular disk files don't need non-blocking treatment

**Next Steps:**

- Read [5_HTTP_PROTOCOL.md](5_HTTP_PROTOCOL.md) to understand HTTP
- Study SocketManager.cpp to see it all in action
- Experiment with `telnet localhost 8080` to see raw TCP

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
