# 🔄 C++ for C Developers

**Transitioning from C to C++ in the Pginx Project**

This guide helps C developers understand the C++ features used in Pginx. We'll focus on **practical examples from our codebase** rather than theoretical concepts.

---

## 📋 Table of Contents

1. [Why C++ in This Project?](#why-c-in-this-project)
2. [Classes and Objects](#classes-and-objects)
3. [References vs Pointers](#references-vs-pointers)
4. [Memory Management and RAII](#memory-management-and-raii)
5. [STL Containers](#stl-containers)
6. [Strings in C++](#strings-in-c)
7. [Namespaces](#namespaces)
8. [Inheritance and Polymorphism](#inheritance-and-polymorphism)
9. [Function Overloading](#function-overloading)
10. [Exception Handling](#exception-handling)
11. [Const Correctness](#const-correctness)
12. [Common Pitfalls](#common-pitfalls)

---

## 🎯 Why C++ in This Project?

The project requirements specify **C++98** for several reasons:

✅ **Better organization** - Classes group related data and functions  
✅ **Type safety** - Stronger typing prevents many errors  
✅ **STL containers** - vector, map, string (no manual memory management!)  
✅ **RAII** - Automatic resource cleanup  
✅ **Polymorphism** - Clean code for different HTTP methods

**Important:** We use **C++98**, not modern C++. No `auto`, `nullptr`, `std::unique_ptr`, etc.

---

## 🏗️ Classes and Objects

### In C: Structs with Function Pointers

```c
// C style
typedef struct {
    char* host;
    int port;
    char* buffer;
} Server;

void server_init(Server* s, const char* host, int port) {
    s->host = strdup(host);
    s->port = port;
    s->buffer = malloc(1024);
}

void server_destroy(Server* s) {
    free(s->host);
    free(s->buffer);
}

// Usage
Server s;
server_init(&s, "localhost", 8080);
// ... use server ...
server_destroy(&s);
```

### In C++: Classes with Methods

```cpp
// C++ style (from our Server class)
class Server : public BaseBlock {
private:
    std::vector<ListenCtx> _listens;        // Private data
    std::vector<std::string> _serverNames;
    std::string _root;

public:
    Server();  // Constructor (replaces init)
    ~Server(); // Destructor (replaces destroy)

    // Methods operate on 'this' object implicitly
    void insertListen(u_int16_t port, const std::string& addr);
    const std::vector<ListenCtx>& getListens() const;
};

// Usage
Server s;  // Constructor called automatically
s.insertListen(8080, "localhost");
// ... use server ...
// Destructor called automatically when s goes out of scope
```

### Key Differences

| C                               | C++                              |
| ------------------------------- | -------------------------------- |
| `struct` with data only         | `class` with data + methods      |
| Separate init/destroy functions | Constructor/destructor           |
| Pass struct pointer everywhere  | Methods access `this` implicitly |
| Manual memory management        | Automatic cleanup                |

### Access Specifiers

```cpp
class Example {
private:    // Only accessible within this class
    int _secretData;

protected:  // Accessible in this class and derived classes
    int _protectedData;

public:     // Accessible everywhere
    int publicData;

    void publicMethod() {
        _secretData = 42;  // OK: we're inside the class
    }
};

// Usage
Example e;
e.publicData = 10;     // OK
e.publicMethod();      // OK
e._secretData = 20;    // ERROR: private!
```

**Convention in Pginx:** Private members start with underscore (`_listens`, `_root`)

---

## 📌 References vs Pointers

### Pointers (from C)

```cpp
void modify(int* ptr) {
    if (ptr != NULL) {
        *ptr = 42;
    }
}

int x = 10;
modify(&x);  // x is now 42
```

### References (C++ feature)

```cpp
void modify(int& ref) {
    ref = 42;  // No dereferencing needed!
}

int x = 10;
modify(x);  // x is now 42 (no & needed)
```

### Key Differences

| Pointers                | References              |
| ----------------------- | ----------------------- |
| Can be NULL             | Cannot be NULL          |
| Can be reassigned       | Cannot be rebound       |
| Need `*` to dereference | Automatic dereferencing |
| Use `->` for members    | Use `.` for members     |

### In Our Codebase

```cpp
// From HttpRequest.hpp
class HttpRequest {
protected:
    const RequestContext& _ctx;  // Reference to context

public:
    HttpRequest(const RequestContext& ctx)  // Pass by const reference
        : _ctx(ctx) {}  // Initialize in constructor

    const std::string& getMethod() const;  // Return by const reference
};
```

**Why references?**

- **Efficiency:** No copy (important for large objects like `std::string`)
- **Safety:** Can't be NULL
- **Clarity:** No pointer syntax needed

**When to use what:**

- **`const T&`** for read-only parameters (no copy, can't modify)
- **`T&`** for output parameters (can modify caller's object)
- **`T*`** when NULL is a valid value or need to reassign

---

## 💾 Memory Management and RAII

### RAII: Resource Acquisition Is Initialization

**Core idea:** Constructor acquires resources, destructor releases them.

### In C: Manual Cleanup

```c
FILE* f = fopen("file.txt", "r");
if (f == NULL) return -1;

char* buffer = malloc(1024);
if (buffer == NULL) {
    fclose(f);  // Don't forget!
    return -1;
}

// ... use resources ...

free(buffer);  // Must remember to free
fclose(f);     // Must remember to close
```

**Problems:**

- Easy to forget cleanup
- Multiple exit paths need duplicate cleanup
- Errors lead to leaks

### In C++: RAII

```cpp
class File {
private:
    FILE* _fp;

public:
    File(const char* path) : _fp(fopen(path, "r")) {
        if (_fp == NULL) {
            throw std::runtime_error("Cannot open file");
        }
    }

    ~File() {
        if (_fp != NULL) {
            fclose(_fp);  // Automatic cleanup!
        }
    }

    // Prevent copying
    File(const File&);  // Private, not implemented
    File& operator=(const File&);  // Private, not implemented
};

// Usage
{
    File f("file.txt");  // Constructor opens file
    // ... use file ...
}  // Destructor closes file automatically, even if exception thrown!
```

### In Our Codebase

```cpp
// From SocketManager.hpp
class SocketManager {
private:
    std::vector<int> listeningSockets;
    HttpParser* httpParser;
    HttpResponse* responseBuilder;

public:
    SocketManager()
        : httpParser(new HttpParser()),
          responseBuilder(new HttpResponse()) {
        // Resources acquired
    }

    ~SocketManager() {
        closeSocket();  // Close all sockets
        delete httpParser;  // Free allocated memory
        delete responseBuilder;
        // All cleanup happens automatically!
    }
};
```

**Benefits:** ✅ No memory leaks  
✅ Exception-safe  
✅ Automatic cleanup  
✅ Clear ownership

---

## 📦 STL Containers

STL (Standard Template Library) provides containers that manage memory automatically.

### std::vector - Dynamic Array

```c
// C style
int* array = malloc(10 * sizeof(int));
int capacity = 10;
int size = 0;

// Add element
if (size >= capacity) {
    capacity *= 2;
    array = realloc(array, capacity * sizeof(int));
}
array[size++] = 42;

// Don't forget to free!
free(array);
```

```cpp
// C++ style
std::vector<int> array;  // No malloc needed!

array.push_back(42);     // Grows automatically
array.push_back(100);

int first = array[0];           // Access by index
int size = array.size();        // Get size

// Memory freed automatically when vector destroyed
```

### From Our Codebase

```cpp
// From Server.hpp
class Server : public BaseBlock {
private:
    std::vector<ListenCtx> _listens;
    std::vector<std::string> _serverNames;
    std::vector<LocationConfig> _locations;

public:
    void insertListen(u_int16_t port, const std::string& addr) {
        ListenCtx ctx;
        ctx.port = port;
        ctx.addr = addr;
        _listens.push_back(ctx);  // Add to vector
    }

    const std::vector<ListenCtx>& getListens() const {
        return _listens;
    }
};

// Usage
Server s;
s.insertListen(8080, "0.0.0.0");
s.insertListen(8081, "127.0.0.1");

const std::vector<ListenCtx>& listens = s.getListens();
for (size_t i = 0; i < listens.size(); ++i) {
    std::cout << listens[i].port << std::endl;
}
```

### std::map - Key-Value Dictionary

```c
// C: Need to implement hash table or use library
// Complex, error-prone
```

```cpp
// C++: Built-in!
std::map<std::string, std::string> headers;

headers["Content-Type"] = "text/html";
headers["Content-Length"] = "1234";

std::string ct = headers["Content-Type"];  // "text/html"

if (headers.count("Host")) {
    // Key exists
}

// Iterate
std::map<std::string, std::string>::iterator it;
for (it = headers.begin(); it != headers.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
}
```

### From Our Codebase

```cpp
// From SocketManager.hpp
class SocketManager {
private:
    std::map<int, std::string> requestBuffers;   // fd -> partial request
    std::map<int, time_t> lastActivity;          // fd -> timestamp
    std::map<int, std::string> sendBuffers;      // fd -> response data
};

// Usage
void SocketManager::handleRequest(int fd, int epoll_fd) {
    char buffer[8192];
    ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

    if (n > 0) {
        requestBuffers[fd].append(buffer, n);  // Accumulate data
        lastActivity[fd] = time(NULL);         // Update timestamp
    }
}
```

### Common Container Operations

| Operation | Vector | Map |
| --- | --- | --- |
| Add element | `v.push_back(x)` | `m[key] = value` |
| Remove element | `v.erase(v.begin() + i)` | `m.erase(key)` |
| Get size | `v.size()` | `m.size()` |
| Check if empty | `v.empty()` | `m.empty()` |
| Clear all | `v.clear()` | `m.clear()` |
| Check existence | - | `m.count(key)` or `m.find(key) != m.end()` |

---

## 🔤 Strings in C++

### C Strings

```c
char* str = malloc(100);
strcpy(str, "Hello");
strcat(str, " World");
int len = strlen(str);
free(str);  // Don't forget!
```

### C++ Strings

```cpp
std::string str = "Hello";
str += " World";           // Concatenation
int len = str.length();    // or str.size()
// No free() needed!
```

### From Our Codebase

```cpp
// From HttpParser.cpp
bool HttpParser::parseRequestLine(const std::string& line,
                                  std::string& method,
                                  std::string& path,
                                  std::string& version) {
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos) return false;

    method = line.substr(0, pos1);  // Extract substring

    size_t pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos) return false;

    path = line.substr(pos1 + 1, pos2 - pos1 - 1);
    version = line.substr(pos2 + 1);

    return true;
}
```

### Useful String Operations

```cpp
std::string s = "Hello World";

// Access characters
char c = s[0];              // 'H'
char last = s[s.length()-1]; // 'd'

// Substring
std::string sub = s.substr(0, 5);  // "Hello"

// Find
size_t pos = s.find("World");      // 6
if (pos != std::string::npos) {
    // Found
}

// Compare
if (s == "Hello World") { }
if (s.compare("Other") == 0) { }

// Append
s += " Everyone";
s.append("!");

// C-string conversion
const char* cstr = s.c_str();  // For C functions
```

### String Streams

```cpp
#include <sstream>

// Convert int to string
int n = 42;
std::ostringstream oss;
oss << n;
std::string s = oss.str();  // "42"

// Parse string
std::string input = "123 456 789";
std::istringstream iss(input);
int a, b, c;
iss >> a >> b >> c;  // a=123, b=456, c=789
```

---

## 🔖 Namespaces

Namespaces prevent name conflicts.

```cpp
// Everything in C++ standard library is in std::
std::string s;
std::vector<int> v;
std::cout << "Hello\n";
```

### Without using namespace

```cpp
#include <iostream>
#include <string>

int main() {
    std::string name = "World";
    std::cout << "Hello " << name << std::endl;
}
```

### With using namespace (avoid in headers!)

```cpp
#include <iostream>
#include <string>

using namespace std;  // Now don't need std:: prefix

int main() {
    string name = "World";  // OK
    cout << "Hello " << name << endl;
}
```

**Best Practice in Pginx:**

- **Never** use `using namespace std;` in **header files**
- OK to use in `.cpp` files (but prefer explicit `std::`)

---

## 🔀 Inheritance and Polymorphism

This is the most important OOP feature used in Pginx!

### Inheritance

```cpp
// Base class (from BaseBlock.hpp)
class BaseBlock {
protected:  // Accessible in derived classes
    std::string _root;
    size_t _clientMaxBodySize;
    std::vector<std::string> _indexFiles;
    std::map<u_int16_t, std::string> _errorPages;
    bool _autoIndex;

public:
    void setRoot(const std::string& root);
    const std::string& getRoot() const;
    // ... other methods
};

// Derived class (from Server.hpp)
class Server : public BaseBlock {  // Inherits from BaseBlock
private:
    std::vector<ListenCtx> _listens;  // Server-specific data
    std::vector<std::string> _serverNames;

public:
    // Inherits all public/protected members from BaseBlock
    void insertListen(u_int16_t port, const std::string& addr);
};

// Another derived class (from LocationConfig.hpp)
class LocationConfig : public BaseBlock {  // Also inherits from BaseBlock
private:
    std::string _path;
    std::vector<std::string> _methods;

public:
    void addMethod(const std::string& method);
};
```

**Benefits:**

- Code reuse (don't repeat `_root`, `_indexFiles`, etc.)
- Consistent interface
- Locations can override server defaults

### Polymorphism

**The power of virtual functions!**

```cpp
// From HttpRequest.hpp
class HttpRequest {
protected:
    const RequestContext& _ctx;
    std::string method;
    std::string path;

public:
    HttpRequest(const RequestContext& ctx);
    virtual ~HttpRequest();  // Virtual destructor!

    // Pure virtual functions (must be implemented by subclasses)
    virtual bool validate(std::string& err) const = 0;
    virtual void handle(HttpResponse& res) = 0;
};

// Subclass for GET/HEAD
class GetHeadRequest : public HttpRequest {
public:
    GetHeadRequest(const RequestContext& ctx);
    virtual ~GetHeadRequest();

    virtual bool validate(std::string& err) const;  // Override
    virtual void handle(HttpResponse& res);         // Override
};

// Subclass for POST
class PostRequest : public HttpRequest {
public:
    PostRequest(const RequestContext& ctx);
    virtual ~PostRequest();

    virtual bool validate(std::string& err) const;  // Different implementation
    virtual void handle(HttpResponse& res);         // Different implementation
};

// Subclass for DELETE
class DeleteRequest : public HttpRequest {
public:
    DeleteRequest(const RequestContext& ctx);
    virtual ~DeleteRequest();

    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};
```

### Factory Pattern

```cpp
// From HttpRequest.cpp
HttpRequest* makeRequestByMethod(const std::string& m,
                                 const RequestContext& ctx) {
    if (m == "GET" || m == "HEAD") {
        return new GetHeadRequest(ctx);
    } else if (m == "POST") {
        return new PostRequest(ctx);
    } else if (m == "DELETE") {
        return new DeleteRequest(ctx);
    }
    return NULL;
}
```

### Using Polymorphism

```cpp
// In SocketManager.cpp
void SocketManager::processFullRequest(int fd, int epfd,
                                       const std::string& rawRequest) {
    // Parse request
    HttpRequest* req = httpParser->parseRequest(rawRequest, server);
    if (!req) {
        // Error handling
        return;
    }

    // Validate (calls appropriate subclass method)
    std::string err;
    if (!req->validate(err)) {
        // Send error response
        delete req;
        return;
    }

    // Handle (calls appropriate subclass method)
    HttpResponse res;
    req->handle(res);  // Polymorphic call!
                      // GetHeadRequest::handle() for GET
                      // PostRequest::handle() for POST
                      // DeleteRequest::handle() for DELETE

    // Send response
    sendHttpResponse(fd, epfd, res);

    // Clean up
    delete req;
}
```

**Magic of polymorphism:**

- Write `req->handle(res)` once
- Different behavior based on actual type
- Easy to add new HTTP methods (just add new subclass)
- Clean, maintainable code

---

## 🔁 Function Overloading

C++ allows multiple functions with the same name but different parameters.

```cpp
// In C: Need different names
void print_int(int x);
void print_string(const char* s);
void print_double(double d);

// In C++: Same name, different parameters
void print(int x);
void print(const std::string& s);
void print(double d);

// Usage
print(42);        // Calls print(int)
print("Hello");   // Calls print(const std::string&)
print(3.14);      // Calls print(double)
```

### Constructor Overloading

```cpp
// From LocationConfig.hpp
class LocationConfig : public BaseBlock {
public:
    LocationConfig();                           // Default constructor
    LocationConfig(const std::string& path);    // Constructor with path
    LocationConfig(const LocationConfig& obj);  // Copy constructor
};

// Usage
LocationConfig loc1;                    // Default
LocationConfig loc2("/api");            // With path
LocationConfig loc3(loc2);              // Copy
```

---

## ⚠️ Exception Handling

C++ provides try-catch for error handling (though we use it sparingly in this project).

```c
// C style
int result = doSomething();
if (result < 0) {
    // Handle error
    return -1;
}
```

```cpp
// C++ style
try {
    doSomething();  // Might throw exception
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // Handle error
}
```

### In Our Codebase

```cpp
// From main.cpp
int main(int argc, char** argv) {
    try {
        std::string content = readFile(argv[1]);
        std::vector<Token> tokens = lexer(content);
        Container container = parser(tokens);
        // ... more code ...
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

**When to use exceptions in this project:**

- Configuration parsing errors
- Critical initialization failures
- **Not for** network I/O errors (use return codes)

---

## 🔒 Const Correctness

`const` in C++ is much more powerful than in C.

### Const Member Functions

```cpp
class Server {
private:
    std::string _root;

public:
    // This function promises not to modify the object
    const std::string& getRoot() const {  // const at end!
        return _root;
    }

    // This function can modify the object
    void setRoot(const std::string& root) {
        _root = root;
    }
};

const Server s;
std::string root = s.getRoot();  // OK: const function
s.setRoot("/www");               // ERROR: can't call non-const function on const object
```

### Const References

```cpp
// Pass by const reference (efficient and safe)
void processRequest(const HttpRequest& req) {  // Can't modify req
    std::string method = req.getMethod();      // OK: getMethod() is const
    req.setMethod("POST");                     // ERROR: setMethod() is not const
}
```

### Const Return Values

```cpp
// Return const reference to avoid copying
const std::vector<ListenCtx>& getListens() const {
    return _listens;  // No copy made!
}

// Usage
const std::vector<ListenCtx>& listens = server.getListens();  // Efficient
std::vector<ListenCtx> copy = server.getListens();            // Makes copy if needed
```

---

## ⚡ Common Pitfalls

### 1. Forgetting to Initialize Members

```cpp
// ❌ Bad
class Server {
    int port;  // Uninitialized!

public:
    Server() {
        // port has garbage value!
    }
};

// ✅ Good
class Server {
    int port;

public:
    Server() : port(8080) {  // Initialize in constructor initializer list
    }
};
```

### 2. Forgetting Virtual Destructor

```cpp
// ❌ Bad
class Base {
public:
    ~Base() { }  // Not virtual!
};

class Derived : public Base {
    int* data;
public:
    Derived() : data(new int[100]) { }
    ~Derived() { delete[] data; }  // Won't be called if deleted through Base*!
};

Base* ptr = new Derived();
delete ptr;  // Memory leak! Derived destructor not called!

// ✅ Good
class Base {
public:
    virtual ~Base() { }  // Virtual!
};
// Now delete ptr; works correctly
```

### 3. Copying Objects with Pointers

```cpp
// ❌ Bad
class SocketManager {
    HttpParser* parser;
public:
    SocketManager() : parser(new HttpParser()) { }
    ~SocketManager() { delete parser; }
    // Default copy constructor: shallow copy!
};

SocketManager sm1;
SocketManager sm2 = sm1;  // Both have same parser pointer!
// Destructor called twice on same pointer = crash!

// ✅ Good
class SocketManager {
private:
    HttpParser* parser;

    // Prevent copying
    SocketManager(const SocketManager&);  // Private, not implemented
    SocketManager& operator=(const SocketManager&);  // Private, not implemented

public:
    SocketManager() : parser(new HttpParser()) { }
    ~SocketManager() { delete parser; }
};
```

### 4. Dangling References

```cpp
// ❌ Bad
const std::string& getName() {
    std::string name = "Server";
    return name;  // Returns reference to local variable!
}  // name destroyed here!

std::string s = getName();  // Undefined behavior!

// ✅ Good
std::string getName() {
    std::string name = "Server";
    return name;  // Returns copy (or uses move semantics)
}
```

### 5. Not Checking NULL After new

```cpp
// ❌ Bad
HttpRequest* req = makeRequestByMethod(method, ctx);
req->handle(res);  // Crash if req is NULL!

// ✅ Good
HttpRequest* req = makeRequestByMethod(method, ctx);
if (!req) {
    // Handle error
    return;
}
req->handle(res);
// ...
delete req;
```

---

## 📝 Quick Reference

### C vs C++ Cheat Sheet

| Task | C | C++ |
| --- | --- | --- |
| **Include** | `#include <string.h>` | `#include <cstring>` or `<string>` |
| **I/O** | `printf("x=%d\n", x);` | `std::cout << "x=" << x << std::endl;` |
| **Strings** | `char* s = malloc(...)` | `std::string s` |
| **Arrays** | `int* arr = malloc(...)` | `std::vector<int> arr` |
| **Dictionary** | Implement hash table | `std::map<K, V>` |
| **Cast** | `(int*)ptr` | `static_cast<int*>(ptr)` |
| **Allocation** | `malloc(size)` | `new Type()` |
| **Deallocation** | `free(ptr)` | `delete ptr` |
| **NULL** | `NULL` | `NULL` (or `0`) |

### Key Syntax

```cpp
// Class definition
class ClassName : public BaseClass {
private:
    int _member;
public:
    ClassName();                         // Constructor
    virtual ~ClassName();                // Virtual destructor
    virtual void method() = 0;           // Pure virtual
    const std::string& getter() const;   // Const method
};

// Constructor implementation
ClassName::ClassName() : _member(0) {
    // Constructor body
}

// Method implementation
void ClassName::method() {
    // Method body
}

// Usage
ClassName obj;                           // Stack allocation
ClassName* ptr = new ClassName();        // Heap allocation
delete ptr;                              // Manual delete

std::vector<int> v;                      // Container (auto cleanup)
v.push_back(42);
```

---

## 🎓 Summary

**Key C++ Features Used in Pginx:**

1. **Classes** - Organize data and functions together
2. **Inheritance** - BaseBlock → Server, LocationConfig
3. **Polymorphism** - HttpRequest subclasses (GET, POST, DELETE)
4. **STL Containers** - vector, map, string (auto memory management)
5. **References** - Efficient parameter passing
6. **RAII** - Automatic resource cleanup
7. **Const Correctness** - Read-only access guarantees
8. **Namespaces** - std:: for standard library

**Next Steps:**

- Look at actual code in `src/models/headers/`
- Practice reading class definitions
- Try modifying a simple class (e.g., add a getter/setter)
- Read [4_NETWORK_PROGRAMMING.md](4_NETWORK_PROGRAMMING.md) next

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
