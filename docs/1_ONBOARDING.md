# 🚀 Onboarding Guide - Welcome to Pginx!

Welcome to the **Pginx** project! This document will help you get up to speed quickly and understand how to navigate the project and documentation.

---

## 📋 Table of Contents

1. [What is Pginx?](#what-is-pginx)
2. [Quick Start](#quick-start)
3. [Project Goals](#project-goals)
4. [Documentation Reading Order](#documentation-reading-order)
5. [Your First Tasks](#your-first-tasks)
6. [Getting Help](#getting-help)

---

## 🎯 What is Pginx?

**Pginx** (inspired by NGINX) is a custom HTTP/1.0 web server implementation written in **C++98**. This project is designed to teach you:

- **Network programming** fundamentals (sockets, TCP/IP, I/O multiplexing)
- **HTTP protocol** implementation from scratch
- **Non-blocking I/O** architecture
- **C++ development** practices (for C developers transitioning to C++)
- **Server architecture** and request/response handling

### Key Features

✅ Multi-port listening and virtual hosting  
✅ GET, POST, DELETE HTTP methods  
✅ Static file serving  
✅ File uploads  
✅ CGI execution (PHP, Python, etc.)  
✅ Configuration file parsing (NGINX-style)  
✅ Custom error pages  
✅ Non-blocking I/O with epoll/poll/select  
✅ Request timeout handling

---

## ⚡ Quick Start

### Prerequisites

- **OS:** Debian Linux (or any Linux distribution)
- **Compiler:** g++ with C++98 support
- **Build tool:** GNU Make
- **Optional:** curl, Python/PHP for CGI testing, a web browser

### Building the Project

```bash
# Clone the repository (if not already done)
cd /home/abueskander/Pginx

# Build the project
make

# This will create the 'webserv' executable
```

### Running the Server

```bash
# Run with default configuration
./webserv config/webserv.conf

# Or use another config file
./webserv config/default.conf
```

### Testing the Server

**Option 1: Using a Web Browser**

```
Open your browser and navigate to:
http://localhost:8080
```

**Option 2: Using curl**

```bash
# Simple GET request
curl http://localhost:8080/

# GET with headers
curl -v http://localhost:8080/index.html

# POST request with data
curl -X POST -d "name=value" http://localhost:8080/upload

# DELETE request
curl -X DELETE http://localhost:8080/test_delete.txt
```

**Option 3: Using provided test scripts**

```bash
# Run all tests
./Tests/run_all_tests.sh

# Run specific test suites
./Tests/core_tests.sh
./Tests/post_tests.sh
./Tests/delete_tests.sh
```

### Cleaning Build Artifacts

```bash
# Remove object files
make clean

# Remove object files and executable
make fclean

# Rebuild everything from scratch
make re
```

---

## 🎯 Project Goals

According to the [subject document](SUBJECT.md), this project aims to:

1. **Implement a compliant HTTP server** following HTTP/1.0 (with some 1.1 features)
2. **Master non-blocking I/O** - All socket operations must be non-blocking
3. **Use I/O multiplexing** - Single `epoll()`/`poll()`/`select()` for all operations
4. **Handle multiple connections** simultaneously without crashing
5. **Parse configuration files** similar to NGINX
6. **Support CGI** for dynamic content generation
7. **Never crash** - The server must handle all edge cases gracefully

### Critical Requirements ⚠️

- **One I/O multiplexer:** Use only ONE `epoll()`/`poll()`/`select()` call
- **Non-blocking sockets:** NEVER call `read()`/`write()` without readiness check
- **No errno checking:** Don't rely on errno after read/write operations
- **No crashes:** Handle out-of-memory, invalid input, etc. gracefully
- **Proper timeouts:** Don't let requests hang indefinitely

---

## 📚 Documentation Reading Order

We've organized the documentation to help you learn systematically. Here's the recommended reading order:

### Phase 1: Understanding the Architecture (Priority) 🔥

1. **[SUBJECT.md](SUBJECT.md)** - Read this first to understand project requirements
2. **[2_ARCHITECTURE.md](2_ARCHITECTURE.md)** ⭐ **START HERE FIRST** ⭐
   - High-level system design
   - Component interactions
   - Request/response flow
   - Class relationships

### Phase 2: Learning C++ (If coming from C)

3. **[3_CPP_FOR_C_DEVELOPERS.md](3_CPP_FOR_C_DEVELOPERS.md)**
   - C to C++ transition
   - Classes and objects
   - STL containers
   - References vs pointers
   - RAII and memory management

### Phase 3: Network Programming Fundamentals

4. **[4_NETWORK_PROGRAMMING.md](4_NETWORK_PROGRAMMING.md)**
   - TCP/IP basics
   - Sockets API
   - Non-blocking I/O
   - epoll/poll/select
   - Network error handling

### Phase 4: HTTP Protocol Deep Dive

5. **[5_HTTP_PROTOCOL.md](5_HTTP_PROTOCOL.md)**
   - HTTP request/response structure
   - Methods (GET, POST, DELETE)
   - Headers and status codes
   - Chunked transfer encoding
   - CGI basics

### Phase 5: Codebase Exploration

6. **[6_CODEBASE_GUIDE.md](6_CODEBASE_GUIDE.md)**
   - File structure explained
   - Core classes deep dive
   - Configuration parser
   - Request handling pipeline
   - Testing strategy

### Phase 6: Development Workflow

7. **[7_DEVELOPMENT_GUIDE.md](7_DEVELOPMENT_GUIDE.md)**
   - Build system
   - Debugging techniques
   - Common pitfalls
   - Contributing guidelines

---

## 🎓 Your First Tasks

Here's a suggested learning path for your first week:

### Day 1-2: Setup and Understanding

- [ ] Read the [SUBJECT.md](SUBJECT.md) document thoroughly
- [ ] Build and run the server successfully
- [ ] Test basic GET requests with curl and browser
- [ ] Read [2_ARCHITECTURE.md](2_ARCHITECTURE.md) to understand the big picture
- [ ] Review the main.cpp to see how everything starts

### Day 3-4: C++ Crash Course (if needed)

- [ ] Read [3_CPP_FOR_C_DEVELOPERS.md](3_CPP_FOR_C_DEVELOPERS.md)
- [ ] Study the class hierarchy (BaseBlock → Server, LocationConfig)
- [ ] Understand how constructors/destructors work in our codebase
- [ ] Learn about STL containers we use (vector, map, string)

### Day 5-7: Network and HTTP Fundamentals

- [ ] Read [4_NETWORK_PROGRAMMING.md](4_NETWORK_PROGRAMMING.md)
- [ ] Understand how epoll works in SocketManager
- [ ] Read [5_HTTP_PROTOCOL.md](5_HTTP_PROTOCOL.md)
- [ ] Trace a request through the codebase using a debugger
- [ ] Study HttpParser and how it extracts data from raw requests

### Week 2: Deep Dive into Code

- [ ] Read [6_CODEBASE_GUIDE.md](6_CODEBASE_GUIDE.md) completely
- [ ] Pick a simple feature (e.g., adding a new header)
- [ ] Implement it and test it
- [ ] Review your changes with the team

### Week 3: Contributing

- [ ] Read [7_DEVELOPMENT_GUIDE.md](7_DEVELOPMENT_GUIDE.md)
- [ ] Pick a TODO item or bug from the issue tracker
- [ ] Implement and test your fix
- [ ] Submit for code review

---

## 🏗️ Project Structure Overview

```
Pginx/
├── config/              # Configuration files (NGINX-style)
├── src/                 # Source code
│   ├── main.cpp        # Entry point
│   ├── models/         # Core classes
│   │   ├── headers/    # Header files
│   │   └── srcs/       # Implementation files
│   └── utils.cpp       # Utility functions
├── includes/           # Common headers
├── www/                # Web content (served files)
├── Tests/              # Test scripts
├── docs/               # Documentation (you are here!)
├── Makefile            # Build configuration
└── webserv             # Compiled executable (after build)
```

---

## 🆘 Getting Help

### When You're Stuck

1. **Read the relevant documentation section** - We've tried to cover everything!
2. **Check the code comments** - Many complex parts are documented inline
3. **Use a debugger** - `gdb` is your friend for tracing execution
4. **Ask the team** - Your teammates are familiar with the context
5. **Compare with NGINX** - When in doubt, see how NGINX behaves
6. **Read the RFCs** - RFC 2616 (HTTP/1.1) and RFC 1945 (HTTP/1.0)

### Useful Commands

```bash
# Debug with GDB
gdb ./webserv
(gdb) run config/webserv.conf
(gdb) break SocketManager::handleClients
(gdb) continue

# Check for memory leaks
valgrind --leak-check=full ./webserv config/webserv.conf

# Monitor system calls
strace ./webserv config/webserv.conf

# Test with telnet (raw HTTP)
telnet localhost 8080
GET / HTTP/1.1
Host: localhost
[press Enter twice]
```

### Common Issues and Solutions

| Issue | Solution |
| --- | --- |
| `Address already in use` | Another process is using the port. Kill it or change the port in config |
| `Segmentation fault` | Use `gdb` to find the crash location. Check for null pointers |
| `Connection refused` | Server might not be running or listening on wrong port |
| `403 Forbidden` | Check file permissions (`chmod 644 file`) |
| `404 Not Found` | Verify the file exists in the configured root directory |

---

## 🎉 Welcome Aboard!

You're now ready to start contributing to Pginx! Remember:

- **Don't hesitate to ask questions** - We're all learning
- **Test your changes thoroughly** - Break things in dev, not in production
- **Read code before writing** - Understanding comes before implementation
- **Take breaks** - Network programming can be intense!

**Next Step:** Read [2_ARCHITECTURE.md](2_ARCHITECTURE.md) to understand how everything fits together!

Good luck, and happy coding! 🚀

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
