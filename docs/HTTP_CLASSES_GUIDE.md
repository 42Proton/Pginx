# HTTP/1.1 Classes - Integration Guide

## Overview
This document describes the HTTP/1.1 request/response classes that have been added to your Pginx project. These classes follow strict C++98 standards and are ready to be integrated into your existing `SocketManager`.

## Available Classes

### 1. HttpRequest (Base Class)
**Location:** `src/models/headers/HttpRequest.hpp`

Base class for all HTTP request types with the following features:

#### Key Methods:
- `const std::string& getMethod()` - Returns HTTP method (GET, POST, etc.)
- `const std::string& getPath()` - Returns request path (without query string)
- `const std::string& getVersion()` - Returns HTTP version
- `const std::map<std::string, std::string>& getHeaders()` - Returns all headers (keys are lowercase)
- `const std::string& getBody()` - Returns request body
- `const std::map<std::string, std::string>& getQuery()` - Returns parsed query parameters
- `bool isChunked()` - Checks if request uses chunked transfer encoding
- `size_t contentLength()` - Returns Content-Length value or 0
- `virtual bool validate(std::string& err)` - Validates request (override in subclasses)
- `virtual void handle(HttpResponse& res)` - Handles request logic (override in subclasses)

#### Static Helper Methods:
- `static void parseQuery(const std::string& target, std::string& cleanPath, std::map<std::string, std::string>& outQuery)` - Parses query string from URL
- `static bool parseHeaderLine(const std::string& line, std::string& k, std::string& v)` - Parses a single header line

### 2. HttpRequest Subclasses
Each HTTP method has its own subclass with specific validation:

- **GetRequest** - Validates that GET requests have no body
- **HeadRequest** - Validates that HEAD requests have no body
- **PostRequest** - Allows body
- **PutRequest** - Allows body
- **PatchRequest** - Allows body
- **DeleteRequest** - Allows body
- **OptionsRequest** - For CORS preflight requests

#### Factory Function:
```cpp
HttpRequest* makeRequestByMethod(const std::string& method);
```
Creates the appropriate request subclass based on method name. Returns `NULL` if method is not supported.

### 3. HttpResponse
**Location:** `src/models/headers/HttpResponse.hpp`

Simple struct for building HTTP responses:

#### Members:
```cpp
int status;                                    // HTTP status code (default: 200)
std::string reason;                            // Status reason phrase (default: "OK")
std::map<std::string, std::string> headers;   // Response headers
std::string body;                              // Response body
```

#### Methods:
- `void setHeader(const std::string& k, const std::string& v)` - Sets a response header
- `std::string serialize(bool headOnly)` - Converts response to HTTP/1.1 format string
  - If `headOnly=true`, omits body (for HEAD requests)

### 4. HttpRouter
**Location:** `src/models/headers/HttpRouter.hpp`

Routes requests to handler functions:

#### Handler Function Type:
```cpp
typedef void (*HandlerFn)(const HttpRequest&, HttpResponse&);
```

#### Methods:
- `void add(const std::string& method, const std::string& path, HandlerFn h)` - Register a route
- `bool dispatch(const HttpRequest& req, HttpResponse& res)` - Find and call handler for request
  - Returns `true` if route was found, `false` if not found
- `std::string allowForPath(const std::string& path)` - Returns comma-separated list of allowed methods for a path (for 405 responses)

#### Demo Handlers Included:
- `void handleRoot(const HttpRequest&, HttpResponse&)` - Serves HTML welcome page at "/"
- `void handleEchoGet(const HttpRequest&, HttpResponse&)` - Echoes query params at GET "/echo"
- `void handleEchoPost(const HttpRequest&, HttpResponse&)` - Echoes body at POST "/echo"  
- `void handleOptions(const HttpRequest&, HttpResponse&)` - Handles OPTIONS with CORS headers

### 5. HttpUtils
**Location:** `src/models/headers/HttpUtils.hpp`

Utility functions for string/HTTP operations:

#### String Functions:
- `std::string trim(const std::string& s)` - Trim whitespace
- `std::string toLowerStr(const std::string& s)` - Convert to lowercase
- `std::string urlDecode(const std::string& s)` - URL decode (handles %XX and +)

#### Number Conversion:
- `size_t parseHex(const std::string& s)` - Parse hexadecimal string
- `size_t safeAtoi(const std::string& s)` - Safe string to integer
- `std::string itoa_custom(size_t n)` - Convert size_t to string
- `std::string itoa_int(int n)` - Convert int to string

#### Socket Functions:
- `bool setNonBlocking(int fd)` - Set socket to non-blocking mode

### 6. HttpConnection (Advanced)
**Location:** `src/models/headers/HttpConnection.hpp`

**Note:** This is a complete connection handler with built-in state machine parser. You may NOT need this if you prefer to integrate the classes directly into your existing SocketManager logic.

## Integration Example

Here's how to use these classes in your SocketManager's TODO section:

```cpp
// In SocketManager::handleRequest() where you have the TODO

size_t header_end = requestBuffers[readyServerFd].find("\r\n\r\n");
if (header_end != std::string::npos) {
    std::string rawRequest = requestBuffers[readyServerFd];
    
    // 1. Parse HTTP request manually or use a parser you build
    //    For now, simple example:
    std::string requestLine = rawRequest.substr(0, rawRequest.find("\r\n"));
    
    // Parse method, path, version from request line
    size_t firstSpace = requestLine.find(' ');
    size_t secondSpace = requestLine.find(' ', firstSpace + 1);
    std::string method = requestLine.substr(0, firstSpace);
    std::string target = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    std::string version = requestLine.substr(secondSpace + 1);
    
    // Create appropriate request object
    HttpRequest* req = makeRequestByMethod(method);
    if (req) {
        // Parse target for path and query
        std::string path;
        std::map<std::string, std::string> query;
        HttpRequest::parseQuery(target, path, query);
        
        req->setPath(path);
        req->setVersion(version);
        req->setQuery(query);
        
        // Parse headers (you would loop through all header lines)
        // ... parse header lines and call req->addHeader(key, value) ...
        
        // Set body if present
        // req->appendBody(...);
        
        // 2. Create router and dispatch
        Router router;
        router.add("GET", "/", handleRoot);
        router.add("GET", "/echo", handleEchoGet);
        router.add("POST", "/echo", handleEchoPost);
        
        HttpResponse res;
        if (!router.dispatch(*req, res)) {
            // 404 Not Found
            res.status = 404;
            res.reason = "Not Found";
            res.body = "Not Found";
            res.setHeader("Content-Type", "text/plain");
            res.setHeader("Content-Length", itoa_custom(res.body.size()));
        }
        
        // 3. Serialize and send response
        std::string responseStr = res.serialize(false);
        sendBuffers[readyServerFd] = responseStr;
        
        delete req;
    }
    
    requestBuffers[readyServerFd].clear();
}
```

## Files Structure

```
src/models/headers/
├── HttpRequest.hpp      - Request base class and subclasses
├── HttpResponse.hpp     - Response struct
├── HttpRouter.hpp       - Router for dispatching requests
├── HttpUtils.hpp        - Utility functions
└── HttpConnection.hpp   - (Optional) Full connection handler

src/models/srcs/
├── HttpRequest.cpp      - Request implementations
├── HttpResponse.cpp     - Response implementations
├── HttpRouter.cpp       - Router and demo handlers
├── HttpUtils.cpp        - Utility implementations
└── HttpConnection.cpp   - (Optional) Connection handler
```

## Key Features

✅ **Strict C++98 compliance** - No C++11/14/17 features
✅ **Production quality** - Proper error handling, validation
✅ **Chunked transfer encoding** support via HttpConnection parser
✅ **Query string parsing** - Automatic URL decoding
✅ **Case-insensitive headers** - All header keys stored lowercase
✅ **Router system** - Easy route registration
✅ **Method validation** - GET/HEAD cannot have bodies
✅ **CORS support** - OPTIONS handler with proper headers

## Next Steps

1. Integrate request parsing into your `SocketManager::handleRequest()`
2. Use the Router to dispatch requests to handlers
3. Build responses using HttpResponse
4. Your existing socket management, epoll, and error handling remain unchanged!
