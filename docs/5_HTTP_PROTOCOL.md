# 📡 HTTP Protocol Guide

**Understanding HTTP/1.0 and HTTP/1.1 for Web Server Implementation**

This document explains the HTTP protocol in detail, from basic concepts to implementation specifics for the Pginx server.

---

## 📋 Table of Contents

1. [HTTP Basics](#http-basics)
2. [HTTP Message Structure](#http-message-structure)
3. [HTTP Methods](#http-methods)
4. [HTTP Headers](#http-headers)
5. [HTTP Status Codes](#http-status-codes)
6. [Content Types and MIME](#content-types-and-mime)
7. [Connection Management](#connection-management)
8. [Chunked Transfer Encoding](#chunked-transfer-encoding)
9. [CGI (Common Gateway Interface)](#cgi-common-gateway-interface)
10. [Practical Examples](#practical-examples)

---

## 🌐 HTTP Basics

### What is HTTP?

**HTTP (Hypertext Transfer Protocol)** is an application-layer protocol for transmitting hypermedia documents (like HTML).

**Key Characteristics:**

- **Client-Server** model (browser requests, server responds)
- **Stateless** (each request is independent)
- **Text-based** protocol (human-readable)
- **Request-Response** pattern

### HTTP Versions

| Version      | Year | Key Features                                          |
| ------------ | ---- | ----------------------------------------------------- |
| **HTTP/0.9** | 1991 | Only GET, no headers, single line response            |
| **HTTP/1.0** | 1996 | Methods, headers, status codes, Content-Type          |
| **HTTP/1.1** | 1997 | Persistent connections, chunked encoding, Host header |
| **HTTP/2**   | 2015 | Binary protocol, multiplexing (not in our scope)      |

**For Pginx:** We implement HTTP/1.0 with some HTTP/1.1 features.

### Client-Server Communication

```
Browser                                    Pginx Server
   |                                            |
   |--- "GET /index.html HTTP/1.1" ----------->|
   |    "Host: localhost"                       |
   |    "User-Agent: Mozilla/5.0"               |
   |    [blank line]                            |
   |                                            |
   |                                        [Process]
   |                                        [Read file]
   |                                        [Build response]
   |                                            |
   |<-- "HTTP/1.1 200 OK" ----------------------|
   |    "Content-Type: text/html"               |
   |    "Content-Length: 1234"                  |
   |    [blank line]                            |
   |    "<html>...</html>"                      |
   |                                            |
```

---

## 📨 HTTP Message Structure

### HTTP Request Format

```
METHOD /path/to/resource HTTP/VERSION\r\n    ← Request Line
Header-Name: Header-Value\r\n                ← Headers
Another-Header: Value\r\n
\r\n                                          ← Blank line (CRLF)
[Optional Body]                               ← Body
```

### Example HTTP Request

```http
GET /index.html HTTP/1.1\r\n
Host: localhost:8080\r\n
User-Agent: Mozilla/5.0\r\n
Accept: text/html\r\n
Connection: keep-alive\r\n
\r\n
```

### HTTP Response Format

```
HTTP/VERSION STATUS_CODE STATUS_MESSAGE\r\n    ← Status Line
Header-Name: Header-Value\r\n                  ← Headers
Another-Header: Value\r\n
\r\n                                            ← Blank line
[Body]                                          ← Body
```

### Example HTTP Response

```http
HTTP/1.1 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 52\r\n
Server: Pginx/1.0\r\n
\r\n
<html><body><h1>Hello World!</h1></body></html>
```

### Parsing Rules

**Critical Points:**

1. **Lines end with `\r\n`** (CRLF - Carriage Return + Line Feed)
2. **Headers end with blank line** (`\r\n\r\n`)
3. **Request line format:** `METHOD PATH VERSION`
4. **Header format:** `Name: Value` (colon + space)
5. **Case-insensitive headers** (e.g., `Content-Type` = `content-type`)

### From Our Codebase

```cpp
// From HttpParser.cpp
bool HttpParser::parseRequestLine(const std::string& line,
                                  std::string& method,
                                  std::string& path,
                                  std::string& version) {
    // Line format: "GET /index.html HTTP/1.1"
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos) return false;

    method = line.substr(0, pos1);

    size_t pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos) return false;

    path = line.substr(pos1 + 1, pos2 - pos1 - 1);
    version = line.substr(pos2 + 1);

    return true;
}
```

---

## 🔧 HTTP Methods

HTTP methods (also called "verbs") indicate the desired action.

### GET - Retrieve Resource

**Purpose:** Request a resource from the server.

**Request:**

```http
GET /index.html HTTP/1.1
Host: localhost
```

**Response:**

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234

<html>...</html>
```

**Characteristics:**

- ✅ **Safe** - Should not modify server state
- ✅ **Idempotent** - Multiple identical requests have same effect
- ✅ **Cacheable** - Response can be cached
- ❌ **No body** in request (params in query string)

**Query Parameters:**

```
GET /search?q=hello&limit=10 HTTP/1.1
         ↑
         Query string starts with ?
         Separated by &
```

### POST - Submit Data

**Purpose:** Submit data to be processed by the server.

**Request:**

```http
POST /upload HTTP/1.1
Host: localhost
Content-Type: application/x-www-form-urlencoded
Content-Length: 27

name=John&email=john@ex.com
```

**Response:**

```http
HTTP/1.1 201 Created
Location: /uploads/file123.txt
Content-Length: 0
```

**Characteristics:**

- ❌ **Not safe** - Modifies server state
- ❌ **Not idempotent** - Multiple requests may create multiple resources
- ⚠️ **Not cacheable** (by default)
- ✅ **Has body** - Data in request body

**Common Content Types:**

- `application/x-www-form-urlencoded` - Form data
- `multipart/form-data` - File uploads
- `application/json` - JSON data

### DELETE - Remove Resource

**Purpose:** Delete a resource on the server.

**Request:**

```http
DELETE /files/test.txt HTTP/1.1
Host: localhost
```

**Response:**

```http
HTTP/1.1 204 No Content
```

**Characteristics:**

- ❌ **Not safe** - Modifies server state
- ✅ **Idempotent** - Deleting multiple times has same effect
- ❌ **No body** typically

### HEAD - Get Headers Only

**Purpose:** Same as GET but only returns headers (no body).

**Request:**

```http
HEAD /large-file.zip HTTP/1.1
Host: localhost
```

**Response:**

```http
HTTP/1.1 200 OK
Content-Type: application/zip
Content-Length: 104857600
Last-Modified: Mon, 01 Nov 2025 12:00:00 GMT
```

**Use Case:** Check if file exists or get file size without downloading.

### Method Summary

| Method     | Safe | Idempotent | Request Body | Response Body |
| ---------- | ---- | ---------- | ------------ | ------------- |
| **GET**    | ✅   | ✅         | ❌           | ✅            |
| **POST**   | ❌   | ❌         | ✅           | ✅            |
| **DELETE** | ❌   | ✅         | ❌           | Optional      |
| **HEAD**   | ✅   | ✅         | ❌           | ❌            |
| PUT        | ❌   | ✅         | ✅           | ✅            |
| PATCH      | ❌   | ❌         | ✅           | ✅            |

---

## 📋 HTTP Headers

Headers provide metadata about the request or response.

### Request Headers

| Header | Purpose | Example |
| --- | --- | --- |
| **Host** | Server hostname (required in HTTP/1.1) | `Host: localhost:8080` |
| **User-Agent** | Client software | `User-Agent: Mozilla/5.0` |
| **Accept** | Acceptable response types | `Accept: text/html,application/json` |
| **Content-Type** | Type of body data | `Content-Type: application/json` |
| **Content-Length** | Size of body in bytes | `Content-Length: 1234` |
| **Connection** | Connection management | `Connection: keep-alive` |
| **Transfer-Encoding** | Encoding of message body | `Transfer-Encoding: chunked` |

### Response Headers

| Header | Purpose | Example |
| --- | --- | --- |
| **Content-Type** | Type of response body | `Content-Type: text/html` |
| **Content-Length** | Size of body in bytes | `Content-Length: 1234` |
| **Server** | Server software | `Server: Pginx/1.0` |
| **Location** | Redirect or created resource URL | `Location: /new-path` |
| **Set-Cookie** | Set cookie on client | `Set-Cookie: session=abc123` |

### Header Parsing Example

```cpp
// From HttpRequest.cpp
bool HttpRequest::parseHeaderLine(const std::string& line,
                                  std::string& key,
                                  std::string& value) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }

    key = line.substr(0, colonPos);

    // Skip colon and spaces
    size_t valueStart = colonPos + 1;
    while (valueStart < line.length() && line[valueStart] == ' ') {
        valueStart++;
    }

    value = line.substr(valueStart);
    return true;
}
```

---

## 🎯 HTTP Status Codes

Status codes indicate the result of the request.

### Status Code Categories

```
1xx - Informational  (Request received, continuing)
2xx - Success        (Request successfully processed)
3xx - Redirection    (Further action needed)
4xx - Client Error   (Request has error)
5xx - Server Error   (Server failed to fulfill valid request)
```

### Common Status Codes

| Code    | Name                  | Meaning                                |
| ------- | --------------------- | -------------------------------------- |
| **200** | OK                    | Success                                |
| **201** | Created               | Resource created (POST)                |
| **204** | No Content            | Success but no body to return          |
| **301** | Moved Permanently     | Resource permanently moved             |
| **302** | Found                 | Resource temporarily moved             |
| **400** | Bad Request           | Invalid request syntax                 |
| **403** | Forbidden             | Access denied                          |
| **404** | Not Found             | Resource doesn't exist                 |
| **405** | Method Not Allowed    | Method not supported for this resource |
| **413** | Payload Too Large     | Request body too large                 |
| **500** | Internal Server Error | Server encountered an error            |
| **501** | Not Implemented       | Server doesn't support this feature    |
| **503** | Service Unavailable   | Server temporarily unavailable         |

### From Our Codebase

```cpp
// From HttpResponse.cpp
void HttpResponse::setError(int code, const std::string& reason) {
    setStatus(code, reason);

    std::ostringstream content;
    content << "<html><body><h1>Error " << code
            << " - " << reason << "</h1></body></html>";
    setBody(content.str());

    std::ostringstream lenStream;
    lenStream << body.size();
    setHeader("Content-Length", lenStream.str());
    setHeader("Content-Type", "text/html");
}
```

---

## 🗂️ Content Types and MIME

MIME (Multipurpose Internet Mail Extensions) types identify content format.

### Common MIME Types

| Extension | MIME Type                | Description    |
| --------- | ------------------------ | -------------- |
| `.html`   | `text/html`              | HTML document  |
| `.css`    | `text/css`               | CSS stylesheet |
| `.js`     | `application/javascript` | JavaScript     |
| `.json`   | `application/json`       | JSON data      |
| `.txt`    | `text/plain`             | Plain text     |
| `.jpg`    | `image/jpeg`             | JPEG image     |
| `.png`    | `image/png`              | PNG image      |
| `.gif`    | `image/gif`              | GIF image      |
| `.pdf`    | `application/pdf`        | PDF document   |
| `.zip`    | `application/zip`        | ZIP archive    |

### Setting Content-Type

```cpp
// Simple file extension to MIME type mapping
std::string getMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";  // Default binary
    }

    std::string ext = path.substr(dotPos);

    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".txt") return "text/plain";

    return "application/octet-stream";
}
```

---

## 🔄 Connection Management

### HTTP/1.0 - Close After Each Request

```http
GET /index.html HTTP/1.0

HTTP/1.0 200 OK
Connection: close
...

[Connection closed]
```

**Problem:** Need to establish new TCP connection for each request (slow!)

### HTTP/1.1 - Persistent Connections

```http
GET /index.html HTTP/1.1
Connection: keep-alive

HTTP/1.1 200 OK
Connection: keep-alive
...

GET /style.css HTTP/1.1
[Same connection reused]

HTTP/1.1 200 OK
...
```

**Benefit:** Reuse TCP connection for multiple requests (faster!)

### Implementation

```cpp
// Check if connection should be kept alive
bool shouldKeepAlive(const HttpRequest& req) {
    const std::map<std::string, std::string>& headers = req.getHeaders();

    std::map<std::string, std::string>::const_iterator it =
        headers.find("Connection");

    if (it != headers.end()) {
        if (it->second == "close") {
            return false;
        }
        if (it->second == "keep-alive") {
            return true;
        }
    }

    // HTTP/1.1 default is keep-alive
    if (req.getVersion() == "HTTP/1.1") {
        return true;
    }

    // HTTP/1.0 default is close
    return false;
}
```

---

## 📦 Chunked Transfer Encoding

When you don't know the content length in advance, use chunked encoding.

### Format

```
Transfer-Encoding: chunked\r\n
\r\n
<size in hex>\r\n
<data>\r\n
<size in hex>\r\n
<data>\r\n
0\r\n
\r\n
```

### Example

```http
HTTP/1.1 200 OK
Transfer-Encoding: chunked

5\r\n
Hello\r\n
6\r\n
 World\r\n
0\r\n
\r\n
```

This sends: "Hello World"

### Parsing Chunked Encoding

```cpp
std::string dechunkBody(const std::string& chunkedBody) {
    std::string result;
    size_t pos = 0;

    while (pos < chunkedBody.length()) {
        // Read chunk size (hex number)
        size_t crlfPos = chunkedBody.find("\r\n", pos);
        if (crlfPos == std::string::npos) break;

        std::string sizeStr = chunkedBody.substr(pos, crlfPos - pos);
        size_t chunkSize = std::strtol(sizeStr.c_str(), NULL, 16);

        if (chunkSize == 0) break;  // Last chunk

        // Read chunk data
        pos = crlfPos + 2;  // Skip \r\n
        result.append(chunkedBody.substr(pos, chunkSize));
        pos += chunkSize + 2;  // Skip chunk data and \r\n
    }

    return result;
}
```

---

## 🖥️ CGI (Common Gateway Interface)

CGI allows the web server to execute external programs and return their output.

### How CGI Works

```
Client                  Pginx              CGI Script (PHP/Python)
  |                       |                        |
  |--- GET /script.php -->|                        |
  |                       |                        |
  |                       |--- fork() ------------>|
  |                       |--- exec("php-cgi") --->|
  |                       |                    [Execute]
  |                       |                    [Generate HTML]
  |                       |<-- stdout -------------|
  |                       |                        |
  |<-- HTTP Response -----|                        |
  |    (with CGI output)  |                        |
```

### CGI Environment Variables

The server must set these environment variables for the CGI script:

```bash
REQUEST_METHOD=GET              # HTTP method
QUERY_STRING=name=value         # Query parameters
CONTENT_LENGTH=123              # Body size (for POST)
CONTENT_TYPE=application/json   # Body type
SCRIPT_FILENAME=/path/to/script.php
PATH_INFO=/extra/path/info
SERVER_PROTOCOL=HTTP/1.1
SERVER_NAME=localhost
SERVER_PORT=8080
REMOTE_ADDR=127.0.0.1
```

### CGI Example

```cpp
// Execute CGI script
void executeCGI(const std::string& scriptPath,
                const HttpRequest& req,
                HttpResponse& res) {
    int pipeFd[2];
    if (pipe(pipeFd) == -1) {
        res.setError(500, "Internal Server Error");
        return;
    }

    pid_t pid = fork();

    if (pid == 0) {  // Child process
        close(pipeFd[0]);  // Close read end

        // Redirect stdout to pipe
        dup2(pipeFd[1], STDOUT_FILENO);
        close(pipeFd[1]);

        // Set environment variables
        setenv("REQUEST_METHOD", req.getMethod().c_str(), 1);
        setenv("QUERY_STRING", extractQuery(req.getPath()).c_str(), 1);
        // ... set more env vars ...

        // Execute CGI
        char* args[] = {(char*)"php-cgi", (char*)scriptPath.c_str(), NULL};
        execve("/usr/bin/php-cgi", args, environ);

        exit(1);  // execve failed

    } else if (pid > 0) {  // Parent process
        close(pipeFd[1]);  // Close write end

        // Read CGI output
        std::string output;
        char buffer[4096];
        ssize_t n;

        while ((n = read(pipeFd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, n);
        }

        close(pipeFd[0]);
        waitpid(pid, NULL, 0);

        // Parse CGI output (headers + body)
        parseCGIOutput(output, res);
    }
}
```

### CGI Output Format

```
Content-Type: text/html\r\n
\r\n
<html><body>Generated content</body></html>
```

CGI scripts can output headers followed by body, or just the body (server adds headers).

---

## 💡 Practical Examples

### Example 1: Simple GET Request

**Request:**

```http
GET /index.html HTTP/1.1
Host: localhost:8080
User-Agent: curl/7.68.0
Accept: */*

```

**Response:**

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 52
Server: Pginx/1.0

<html><body><h1>Welcome!</h1></body></html>
```

### Example 2: POST File Upload

**Request:**

```http
POST /upload HTTP/1.1
Host: localhost:8080
Content-Type: text/plain
Content-Length: 13

Hello, World!
```

**Response:**

```http
HTTP/1.1 201 Created
Location: /uploads/file_12345.txt
Content-Length: 0

```

### Example 3: DELETE Request

**Request:**

```http
DELETE /files/test.txt HTTP/1.1
Host: localhost:8080

```

**Response:**

```http
HTTP/1.1 204 No Content

```

### Example 4: 404 Error

**Request:**

```http
GET /nonexistent.html HTTP/1.1
Host: localhost:8080

```

**Response:**

```http
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: 65

<html><body><h1>Error 404 - Not Found</h1></body></html>
```

---

## 🎓 Summary

**Key HTTP Concepts:**

1. **Request-Response Pattern** - Client asks, server answers
2. **Text-Based Protocol** - Human-readable (except body)
3. **Stateless** - Each request is independent
4. **Methods** - GET (read), POST (create), DELETE (remove)
5. **Status Codes** - 2xx success, 4xx client error, 5xx server error
6. **Headers** - Metadata about request/response
7. **Content-Type** - Identifies body format
8. **Chunked Encoding** - Transfer data without knowing size
9. **CGI** - Execute external programs to generate dynamic content

**Implementation Checklist:**

- ✅ Parse request line (method, path, version)
- ✅ Parse headers (key: value pairs)
- ✅ Handle body (based on Content-Length or chunked)
- ✅ Route to appropriate handler (GET/POST/DELETE)
- ✅ Build response (status, headers, body)
- ✅ Set correct Content-Type
- ✅ Handle errors gracefully
- ✅ Support CGI execution

**Next Steps:**

- Read [6_CODEBASE_GUIDE.md](6_CODEBASE_GUIDE.md) to see how we implement all of this
- Test with `curl` and `telnet` to understand HTTP at wire level
- Read RFC 2616 (HTTP/1.1) for complete specification

---

**Document Version:** 1.0  
**Last Updated:** November 2025  
**Maintained by:** Pginx Team
