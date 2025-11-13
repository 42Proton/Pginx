# 🛠️ Development Guide

**Building, Testing, Debugging, and Contributing to Pginx**

This guide covers the practical aspects of development: building, testing, debugging, and common workflows.

---

## 📋 Table of Contents

1. [Development Environment Setup](#development-environment-setup)
2. [Build System](#build-system)
3. [Testing](#testing)
4. [Debugging](#debugging)
5. [Common Development Workflows](#common-development-workflows)
6. [Code Style Guidelines](#code-style-guidelines)
7. [Git Workflow](#git-workflow)
8. [Common Issues and Solutions](#common-issues-and-solutions)
9. [Performance Profiling](#performance-profiling)
10. [Contributing Guidelines](#contributing-guidelines)

---

## 💻 Development Environment Setup

### Required Tools

```bash
# Check if you have required tools
g++ --version          # Should be 4.x or later (with C++98 support)
make --version         # GNU Make
gdb --version          # GNU Debugger
valgrind --version     # Memory leak detector
curl --version         # HTTP testing tool

# Install missing tools (Debian/Ubuntu)
sudo apt update
sudo apt install build-essential gdb valgrind curl
```

### Optional but Recommended

```bash
# Install useful development tools
sudo apt install \
    python3 \
    php-cgi \
    siege \
    apache2-utils \
    net-tools
```

### IDE/Editor Setup

**VS Code Extensions:**

- C/C++ (Microsoft)
- Makefile Tools
- GitLens
- Error Lens

**Vim/Neovim:**

```vim
" Add to .vimrc
syntax on
set number
set tabstop=4
set shiftwidth=4
set expandtab
```

---

## 🔨 Build System

### Makefile Structure

**Path:** `Makefile`

```makefile
# Executable name
NAME = webserv

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I./includes -I./src/models/headers

# Source files
SRCS = src/main.cpp \
       src/utils.cpp \
       src/models/srcs/SocketManager.cpp \
       src/models/srcs/HttpParser.cpp \
       src/models/srcs/HttpRequest.cpp \
       src/models/srcs/HttpResponse.cpp \
       # ... more files ...

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(NAME)

# Link
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJS)

# Clean everything
fclean: clean
	rm -f $(NAME)

# Rebuild
re: fclean all

.PHONY: all clean fclean re
```

### Build Commands

```bash
# Standard build
make

# Rebuild from scratch
make re

# Clean object files only
make clean

# Clean everything
make fclean

# Debug build (add to Makefile)
make debug    # Compiles with -g -O0

# Build with verbose output
make VERBOSE=1
```

### Debug Build

Add this to Makefile:

```makefile
# Debug flags
DEBUG_FLAGS = -g -O0 -DDEBUG

# Debug target
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: re
```

Usage:

```bash
make debug
gdb ./webserv
```

---

## 🧪 Testing

### Test Structure

```
Tests/
├── run_all_tests.sh        # Run all test suites
├── core_tests.sh           # Basic GET/HEAD tests
├── post_tests.sh           # POST and upload tests
├── delete_tests.sh         # DELETE tests
├── error_tests.sh          # Error handling tests
└── parser_tests.sh         # Config parser tests
```

### Running Tests

```bash
# Run all tests
./Tests/run_all_tests.sh

# Run specific test suite
./Tests/core_tests.sh
./Tests/post_tests.sh

# Make tests executable if needed
chmod +x Tests/*.sh
```

### Manual Testing with curl

```bash
# GET request
curl -v http://localhost:8080/

# GET with specific file
curl http://localhost:8080/index.html

# POST data
curl -X POST -d "name=John&age=30" http://localhost:8080/upload

# POST file
curl -X POST -F "file=@test.txt" http://localhost:8080/upload

# DELETE request
curl -X DELETE http://localhost:8080/files/test.txt

# HEAD request (headers only)
curl -I http://localhost:8080/

# Test with custom headers
curl -H "Custom-Header: value" http://localhost:8080/

# Follow redirects
curl -L http://localhost:8080/redirect

# Save response to file
curl -o output.html http://localhost:8080/
```

### Manual Testing with telnet

```bash
# Connect to server
telnet localhost 8080

# Type HTTP request manually
GET / HTTP/1.1
Host: localhost
[Press Enter twice]

# You'll see the raw HTTP response
```

### Browser Testing

```bash
# Open in browser
firefox http://localhost:8080/
chromium http://localhost:8080/

# Test upload form
firefox http://localhost:8080/upload_form.html
```

### Stress Testing

```bash
# Apache Bench - 1000 requests, 10 concurrent
ab -n 1000 -c 10 http://localhost:8080/

# Siege - continuous requests
siege -c 10 -t 30s http://localhost:8080/

# Custom script
for i in {1..100}; do
    curl http://localhost:8080/ &
done
wait
```

### Writing Test Scripts

**Example:** `Tests/custom_test.sh`

```bash
#!/bin/bash

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Test counter
TESTS=0
PASSED=0

# Helper function
test_request() {
    TESTS=$((TESTS + 1))
    local name="$1"
    local expected_code="$2"
    local url="$3"

    actual_code=$(curl -s -o /dev/null -w "%{http_code}" "$url")

    if [ "$actual_code" -eq "$expected_code" ]; then
        echo -e "${GREEN}✓${NC} $name"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗${NC} $name (expected $expected_code, got $actual_code)"
    fi
}

# Run tests
test_request "GET index" 200 "http://localhost:8080/"
test_request "GET nonexistent" 404 "http://localhost:8080/nope.html"
test_request "POST upload" 201 "http://localhost:8080/upload" -X POST -d "data=test"

# Summary
echo ""
echo "Tests: $PASSED/$TESTS passed"

if [ $PASSED -eq $TESTS ]; then
    exit 0
else
    exit 1
fi
```

---

## 🐛 Debugging

### Using GDB

```bash
# Start with GDB
gdb ./webserv

# Common GDB commands
(gdb) run config/webserv.conf              # Run program
(gdb) break SocketManager::handleClients   # Set breakpoint
(gdb) break main.cpp:42                    # Break at line
(gdb) continue                             # Continue execution
(gdb) next                                 # Step over (next line)
(gdb) step                                 # Step into (enter function)
(gdb) print variable                       # Print variable value
(gdb) backtrace                            # Show call stack
(gdb) info locals                          # Show local variables
(gdb) quit                                 # Exit GDB
```

### Debugging a Specific Request

```bash
# Terminal 1: Start server in GDB
gdb ./webserv
(gdb) break HttpParser::parseRequest
(gdb) run config/webserv.conf

# Terminal 2: Send request
curl http://localhost:8080/

# Back to Terminal 1: GDB will break at parseRequest
(gdb) print rawRequest
(gdb) next
(gdb) print method
(gdb) continue
```

### Memory Leak Detection with Valgrind

```bash
# Check for memory leaks
valgrind --leak-check=full --show-leak-kinds=all ./webserv config/webserv.conf

# More detailed output
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./webserv config/webserv.conf

# Send some requests, then Ctrl+C the server
# Check valgrind-out.txt for leaks
```

### Common Valgrind Issues

```
LEAK SUMMARY:
   definitely lost: 0 bytes in 0 blocks     ✅ Good
   indirectly lost: 0 bytes in 0 blocks     ✅ Good
   possibly lost: 0 bytes in 0 blocks       ✅ Good
   still reachable: 72 bytes in 1 blocks    ⚠️ OK (might be caching)
```

### Logging and Debugging Output

```cpp
// Add debug macro
#ifdef DEBUG
#define DEBUG_LOG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
#define DEBUG_LOG(msg)
#endif

// Usage
DEBUG_LOG("Processing request from fd=" << fd);
DEBUG_LOG("Method: " << method << ", Path: " << path);
```

Compile with debug:

```bash
make debug
./webserv config/webserv.conf
```

### Network Debugging

```bash
# Monitor network traffic
sudo tcpdump -i lo -A port 8080

# See what's listening on ports
netstat -tuln | grep 8080
ss -tuln | grep 8080

# Check open files by process
lsof -i :8080

# Send raw HTTP with netcat
nc localhost 8080
GET / HTTP/1.1
Host: localhost

```

---

## 🔄 Common Development Workflows

### Adding a New HTTP Method

**Example: Adding PATCH support**

1. **Add to HttpRequest.hpp**

```cpp
class PatchRequest : public HttpRequest {
public:
    PatchRequest(const RequestContext& ctx);
    virtual ~PatchRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};
```

2. **Implement in HttpRequest.cpp**

```cpp
PatchRequest::PatchRequest(const RequestContext& ctx)
    : HttpRequest(ctx) {}

bool PatchRequest::validate(std::string& err) const {
    // Validation logic
    return true;
}

void PatchRequest::handle(HttpResponse& res) {
    // Handle PATCH request
}
```

3. **Add to factory**

```cpp
HttpRequest* makeRequestByMethod(const std::string& m,
                                 const RequestContext& ctx) {
    // ... existing methods ...
    else if (m == "PATCH") {
        return new PatchRequest(ctx);
    }
}
```

4. **Test**

```bash
make re
./webserv config/webserv.conf

# In another terminal
curl -X PATCH -d "field=value" http://localhost:8080/resource
```

### Adding a Configuration Directive

**Example: Adding `client_timeout` directive**

1. **Add to BaseBlock.hpp**

```cpp
class BaseBlock {
protected:
    int _clientTimeout;  // seconds

public:
    void setClientTimeout(int timeout);
    int getClientTimeout() const;
};
```

2. **Implement in BaseBlock.cpp**

```cpp
void BaseBlock::setClientTimeout(int timeout) {
    _clientTimeout = timeout;
}

int BaseBlock::getClientTimeout() const {
    return _clientTimeout;
}
```

3. **Update parser to recognize directive**

```cpp
// In parser.cpp
if (token.value == "client_timeout") {
    int timeout = parseIntValue(tokens[++i]);
    currentBlock.setClientTimeout(timeout);
}
```

4. **Use in SocketManager**

```cpp
void SocketManager::handleTimeouts(int epoll_fd) {
    time_t now = time(NULL);
    int timeout = _ctx.getServer().getClientTimeout();

    // ... check if client exceeded timeout ...
}
```

5. **Test in config**

```nginx
server {
    client_timeout 30;
    # ...
}
```

### Fixing a Bug

1. **Reproduce the bug**

```bash
# Document steps to reproduce
curl http://localhost:8080/trigger-bug
```

2. **Write a test that fails**

```bash
# Add to Tests/bug_fix_test.sh
test_request "Bug #42 - Should return 200" 200 "http://localhost:8080/specific-case"
```

3. **Debug**

```bash
gdb ./webserv
(gdb) break SuspectedFunction
(gdb) run
# Trigger the bug
(gdb) print variables
```

4. **Fix the code**

5. **Verify fix**

```bash
make re
./Tests/bug_fix_test.sh
./Tests/run_all_tests.sh
```

6. **Commit**

```bash
git add <files>
git commit -m "Fix: Issue #42 - Incorrect handling of edge case"
```

---

## 📝 Code Style Guidelines

### Naming Conventions

```cpp
// Classes: PascalCase
class HttpRequest { };
class SocketManager { };

// Functions: camelCase
void handleRequest();
bool validateInput();

// Private members: _camelCase (underscore prefix)
class Server {
private:
    std::string _root;
    std::vector<int> _ports;
};

// Constants: UPPER_SNAKE_CASE
#define MAX_CONNECTIONS 1000
const int BUFFER_SIZE = 4096;

// Local variables: camelCase
int clientFd = accept(...);
std::string requestData;
```

### Formatting

```cpp
// Braces on new line
void function()
{
    if (condition)
    {
        // code
    }
}

// Indentation: 4 spaces (no tabs)
void example()
{
    if (condition)
    {
        for (int i = 0; i < 10; i++)
        {
            doSomething();
        }
    }
}

// Pointer/reference alignment: type& var
std::string& getRef();
int* getPointer();

// One statement per line
// ✅ Good
int x = 10;
int y = 20;

// ❌ Bad
int x = 10; int y = 20;
```

### Comments

```cpp
// Single-line comments for brief explanations
int timeout = 60;  // seconds

/**
 * Multi-line comments for function/class documentation
 * Explain what the function does, parameters, return value
 */
void complexFunction(int param1, const std::string& param2)
{
    // Implementation
}
```

---

## 🌿 Git Workflow

### Branch Strategy

```
main (or master)
  ├── feature/add-https-support
  ├── bugfix/fix-memory-leak
  └── refactor/improve-parser
```

### Common Git Commands

```bash
# Check status
git status

# Create and switch to new branch
git checkout -b feature/my-feature

# Stage changes
git add file1.cpp file2.hpp
git add src/models/srcs/  # Add directory

# Commit
git commit -m "Add: New feature description"

# Push to remote
git push origin feature/my-feature

# Update from remote
git pull origin main

# Merge branch
git checkout main
git merge feature/my-feature

# View history
git log --oneline --graph

# Discard local changes
git checkout -- file.cpp

# Create a tag
git tag v1.0.0
git push origin v1.0.0
```

### Commit Message Format

```
Type: Brief description (50 chars or less)

More detailed explanation if needed (wrap at 72 chars).
Explain what and why, not how.

Fixes #issue-number
```

**Types:**

- `Add:` New feature
- `Fix:` Bug fix
- `Refactor:` Code restructuring
- `Docs:` Documentation
- `Test:` Tests
- `Style:` Formatting

**Examples:**

```
Add: Support for chunked transfer encoding

Implement parsing and reassembly of chunked HTTP requests
according to RFC 2616 section 3.6.1.

Fixes #42
```

---

## 🔧 Common Issues and Solutions

### Issue: "Address already in use"

**Cause:** Port is still bound from previous run.

**Solution:**

```bash
# Find process using port
lsof -i :8080
# or
netstat -tulpn | grep 8080

# Kill the process
kill -9 <PID>

# Or wait 60 seconds for OS to release port
# Or add SO_REUSEADDR in code (already done)
```

### Issue: "Segmentation fault"

**Solution:**

```bash
# Run with GDB
gdb ./webserv
(gdb) run config/webserv.conf
# When it crashes:
(gdb) backtrace
(gdb) frame 0
(gdb) print variable

# Or use Valgrind
valgrind --track-origins=yes ./webserv config/webserv.conf
```

### Issue: "Connection refused"

**Check:**

```bash
# Is server running?
ps aux | grep webserv

# Is it listening on correct port?
netstat -tuln | grep 8080

# Can you connect locally?
telnet localhost 8080

# Firewall blocking?
sudo ufw status
```

### Issue: Memory leaks

**Solution:**

```bash
# Find leaks
valgrind --leak-check=full ./webserv config/webserv.conf

# Common causes:
# 1. Forgetting to delete dynamically allocated objects
HttpRequest* req = new GetHeadRequest(ctx);
req->handle(res);
delete req;  // Don't forget!

# 2. Not closing file descriptors
int fd = open(...);
// ... use fd ...
close(fd);  // Don't forget!
```

### Issue: Makefile errors

```bash
# Clean and rebuild
make fclean
make

# Check for typos in Makefile
# Ensure proper indentation (tabs, not spaces)
```

---

## 📊 Performance Profiling

### Using `perf` (Linux)

```bash
# Install perf
sudo apt install linux-tools-generic

# Profile the server
sudo perf record -g ./webserv config/webserv.conf

# Generate requests
ab -n 10000 -c 100 http://localhost:8080/

# Stop server (Ctrl+C)

# View results
sudo perf report
```

### Using `time`

```bash
# Measure execution time
time ./webserv config/webserv.conf
```

### Benchmark with Apache Bench

```bash
# 1000 requests, 10 concurrent
ab -n 1000 -c 10 http://localhost:8080/

# Results show:
# - Requests per second
# - Time per request
# - Transfer rate
```

---

## 🤝 Contributing Guidelines

### Before You Start

1. **Read the documentation** (you're doing it!)
2. **Check existing issues** on GitHub
3. **Discuss major changes** with the team first

### Pull Request Process

1. **Create a branch**

   ```bash
   git checkout -b feature/my-feature
   ```

2. **Make your changes**

   - Follow code style guidelines
   - Add tests
   - Update documentation

3. **Test thoroughly**

   ```bash
   make re
   ./Tests/run_all_tests.sh
   valgrind --leak-check=full ./webserv config/webserv.conf
   ```

4. **Commit**

   ```bash
   git add .
   git commit -m "Add: My feature description"
   ```

5. **Push and create PR**

   ```bash
   git push origin feature/my-feature
   # Create pull request on GitHub
   ```

6. **Code review**

   - Address review comments
   - Update PR with changes

7. **Merge**
   - After approval, PR is merged

### Code Review Checklist

- [ ] Code follows style guidelines
- [ ] All tests pass
- [ ] No memory leaks (valgrind)
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] No unnecessary files committed

---

## 🎓 Summary

**Essential Tools:**

- GDB for debugging
- Valgrind for memory leaks
- curl for HTTP testing
- git for version control

**Key Commands:**

```bash
make re                                    # Rebuild
./Tests/run_all_tests.sh                  # Test
gdb ./webserv                             # Debug
valgrind --leak-check=full ./webserv ...  # Check leaks
curl -v http://localhost:8080/            # Test HTTP
```

**Development Cycle:**

1. Write code
2. Build (`make re`)
3. Test (automated + manual)
4. Debug if needed
5. Check for leaks
6. Commit
7. Push

**Remember:**

- Test early and often
- Use debugger instead of printf
- Check for memory leaks before committing
- Write clear commit messages
- Ask for help when stuck!

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
