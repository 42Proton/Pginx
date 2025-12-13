# Auto Index and Cookie Handling Implementation Plan

**Project:** Pginx HTTP Web Server  
**Date:** December 13, 2025  
**Author:** GitHub Copilot  
**Status:** Planning Phase

---

## Table of Contents
1. [Overview](#overview)
2. [Auto Index (Directory Listing)](#auto-index-directory-listing)
3. [HTTP Cookie Handling](#http-cookie-handling)
4. [Implementation Roadmap](#implementation-roadmap)
5. [File Structure](#file-structure)
6. [Testing Strategy](#testing-strategy)

---

## Overview

This document outlines the implementation plan for two important HTTP features:

1. **Auto Index**: Generate HTML directory listings when `autoindex on` is configured
2. **Cookie Handling**: Parse `Cookie` headers from requests and set `Set-Cookie` headers in responses

Both features are essential for modern web applications and session management.

---

## Auto Index (Directory Listing)

### Current State
- ✅ Config parsing for `autoindex` directive is implemented
- ✅ `_autoIndex` flag stored in `BaseBlock` class
- ✅ `getAutoIndex()` method available in `RequestContext`
- ❌ Directory listing HTML generation is **NOT** implemented (TODO at line 264 in HttpRequest.cpp)

### Requirements

When a directory is requested and:
- No index file exists (e.g., `index.html`, `index.htm`)
- `autoindex on` is configured
- The client has read permissions

Then the server should:
1. Read directory contents using `opendir()` and `readdir()`
2. Generate an HTML page listing files and subdirectories
3. Include file metadata (size, modification date)
4. Sort entries (directories first, then files alphabetically)
5. Provide clickable links to navigate
6. Add parent directory `../` link (except at root)

### HTML Output Format

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Index of /path/to/directory</title>
    <style>
        body { font-family: monospace; padding: 20px; }
        h1 { font-size: 1.5em; margin-bottom: 20px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { text-align: left; padding: 8px; }
        th { border-bottom: 2px solid #ddd; }
        tr:hover { background-color: #f5f5f5; }
        a { text-decoration: none; color: #0366d6; }
        a:hover { text-decoration: underline; }
        .dir { font-weight: bold; }
        .size { text-align: right; }
        .date { text-align: right; }
    </style>
</head>
<body>
    <h1>Index of /path/to/directory</h1>
    <table>
        <thead>
            <tr>
                <th>Name</th>
                <th class="size">Size</th>
                <th class="date">Last Modified</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><a href="../" class="dir">../</a></td>
                <td class="size">-</td>
                <td class="date">-</td>
            </tr>
            <!-- Directory entries -->
            <tr>
                <td><a href="subdir/" class="dir">subdir/</a></td>
                <td class="size">-</td>
                <td class="date">2025-12-13 14:30:00</td>
            </tr>
            <!-- File entries -->
            <tr>
                <td><a href="file.txt">file.txt</a></td>
                <td class="size">1.2 KB</td>
                <td class="date">2025-12-13 14:25:00</td>
            </tr>
        </tbody>
    </table>
</body>
</html>
```

### Implementation Details

#### Step 1: Add Utility Functions to HttpUtils

**File:** `src/models/headers/HttpUtils.hpp`

Add declarations:
```cpp
// Directory listing utilities
std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath);
std::string formatFileSize(off_t size);
std::string formatTimestamp(time_t time);
```

**File:** `src/models/srcs/HttpUtils.cpp`

Implement:
```cpp
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <algorithm>

struct DirEntry {
    std::string name;
    bool isDir;
    off_t size;
    time_t mtime;
    
    bool operator<(const DirEntry& other) const {
        // Directories come first
        if (isDir != other.isDir)
            return isDir > other.isDir;
        // Then alphabetically
        return name < other.name;
    }
};

std::string formatFileSize(off_t size) {
    if (size < 1024)
        return itoa_custom(size) + " B";
    if (size < 1024 * 1024)
        return itoa_custom(size / 1024) + " KB";
    if (size < 1024 * 1024 * 1024)
        return itoa_custom(size / (1024 * 1024)) + " MB";
    return itoa_custom(size / (1024 * 1024 * 1024)) + " GB";
}

std::string formatTimestamp(time_t time) {
    char buf[64];
    struct tm* tm_info = localtime(&time);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buf);
}

std::string generateDirectoryListing(const std::string& dirPath, const std::string& requestPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
        return "";
    
    std::vector<DirEntry> entries;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;
        
        std::string fullPath = dirPath + "/" + entry->d_name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == -1)
            continue;
        
        DirEntry de;
        de.name = entry->d_name;
        de.isDir = S_ISDIR(st.st_mode);
        de.size = st.st_size;
        de.mtime = st.st_mtime;
        entries.push_back(de);
    }
    closedir(dir);
    
    // Sort entries
    std::sort(entries.begin(), entries.end());
    
    // Generate HTML
    std::string html = "<!DOCTYPE html>\n<html>\n<head>\n";
    html += "<meta charset=\"UTF-8\">\n";
    html += "<title>Index of " + requestPath + "</title>\n";
    html += "<style>\n";
    html += "body { font-family: monospace; padding: 20px; background: #f9f9f9; }\n";
    html += "h1 { font-size: 1.5em; margin-bottom: 20px; border-bottom: 2px solid #ddd; padding-bottom: 10px; }\n";
    html += "table { border-collapse: collapse; width: 100%; background: white; }\n";
    html += "th, td { text-align: left; padding: 10px; }\n";
    html += "th { background: #e9e9e9; border-bottom: 2px solid #ddd; font-weight: bold; }\n";
    html += "tr:hover { background-color: #f5f5f5; }\n";
    html += "a { text-decoration: none; color: #0366d6; }\n";
    html += "a:hover { text-decoration: underline; }\n";
    html += ".dir::before { content: '📁 '; }\n";
    html += ".file::before { content: '📄 '; }\n";
    html += ".size { text-align: right; }\n";
    html += ".date { text-align: right; color: #666; }\n";
    html += "</style>\n</head>\n<body>\n";
    html += "<h1>Index of " + requestPath + "</h1>\n";
    html += "<table>\n<thead>\n<tr>\n";
    html += "<th>Name</th><th class=\"size\">Size</th><th class=\"date\">Last Modified</th>\n";
    html += "</tr>\n</thead>\n<tbody>\n";
    
    // Add parent directory link (if not root)
    if (requestPath != "/") {
        html += "<tr>\n";
        html += "<td><a href=\"../\" class=\"dir\">../</a></td>\n";
        html += "<td class=\"size\">-</td>\n";
        html += "<td class=\"date\">-</td>\n";
        html += "</tr>\n";
    }
    
    // Add entries
    for (size_t i = 0; i < entries.size(); i++) {
        const DirEntry& e = entries[i];
        html += "<tr>\n";
        
        std::string href = e.name;
        if (e.isDir)
            href += "/";
        
        html += "<td><a href=\"" + href + "\" class=\"" + (e.isDir ? "dir" : "file") + "\">";
        html += e.name;
        if (e.isDir)
            html += "/";
        html += "</a></td>\n";
        
        html += "<td class=\"size\">";
        html += e.isDir ? "-" : formatFileSize(e.size);
        html += "</td>\n";
        
        html += "<td class=\"date\">" + formatTimestamp(e.mtime) + "</td>\n";
        html += "</tr>\n";
    }
    
    html += "</tbody>\n</table>\n</body>\n</html>";
    
    return html;
}
```

#### Step 2: Update HttpRequest.cpp

**File:** `src/models/srcs/HttpRequest.cpp`

Replace the TODO section (around line 264):

```cpp
if (_ctx.getAutoIndex())
{
    // Generate directory listing
    std::string html = generateDirectoryListing(fullPath, path);
    if (html.empty()) {
        res.setErrorFromContext(500, _ctx);
        return;
    }
    
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", "text/html; charset=utf-8");
    res.setBody(html);
    
    std::ostringstream lenStream;
    lenStream << html.length();
    res.setHeader("Content-Length", lenStream.str());
    return;
}
```

---

## HTTP Cookie Handling

### Overview

Cookies are small pieces of data stored by the browser and sent with every request to the same domain. They're essential for:
- **Session management**: User logins, shopping carts
- **Personalization**: User preferences, themes
- **Tracking**: Analytics, advertising (optional)

### HTTP Cookie Mechanism

**Client → Server (Request)**:
```http
GET /profile HTTP/1.1
Host: example.com
Cookie: session_id=abc123; user_pref=dark_mode
```

**Server → Client (Response)**:
```http
HTTP/1.1 200 OK
Set-Cookie: session_id=xyz789; Path=/; HttpOnly; Secure
Set-Cookie: user_pref=light_mode; Max-Age=31536000; Path=/
Content-Type: text/html

<html>...</html>
```

### Cookie Attributes

| Attribute | Purpose | Example |
|-----------|---------|---------|
| **Name=Value** | The actual cookie data | `session_id=abc123` |
| **Domain** | Which domain can access cookie | `Domain=example.com` |
| **Path** | Which paths can access cookie | `Path=/` or `Path=/admin` |
| **Expires** | Absolute expiration date | `Expires=Wed, 13 Dec 2026 12:00:00 GMT` |
| **Max-Age** | Relative expiration in seconds | `Max-Age=3600` (1 hour) |
| **Secure** | Only send over HTTPS | `Secure` |
| **HttpOnly** | Not accessible via JavaScript | `HttpOnly` |
| **SameSite** | CSRF protection | `SameSite=Strict` or `Lax` or `None` |

### Requirements

Our web server needs to:

1. **Parse incoming Cookie headers** from client requests
2. **Store cookies** in a data structure accessible to request handlers
3. **Allow CGI scripts** to read cookies via environment variables
4. **Support Set-Cookie** in responses (both static and CGI)
5. **Validate cookie format** according to RFC 6265

### Implementation Details

#### Step 1: Add Cookie Data Structure

**File:** `src/models/headers/HttpRequest.hpp`

Add a cookie storage member:
```cpp
class HttpRequest {
private:
    // ... existing members ...
    std::map<std::string, std::string> _cookies;  // Cookie name → value

public:
    // ... existing methods ...
    
    // Cookie accessors
    const std::map<std::string, std::string>& getCookies() const;
    std::string getCookie(const std::string& name) const;
    bool hasCookie(const std::string& name) const;
};
```

**File:** `src/models/srcs/HttpRequest.cpp`

Implement accessors:
```cpp
const std::map<std::string, std::string>& HttpRequest::getCookies() const {
    return _cookies;
}

std::string HttpRequest::getCookie(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _cookies.find(name);
    if (it != _cookies.end())
        return it->second;
    return "";
}

bool HttpRequest::hasCookie(const std::string& name) const {
    return _cookies.find(name) != _cookies.end();
}
```

#### Step 2: Parse Cookie Header

**File:** `src/models/headers/HttpUtils.hpp`

Add cookie parsing function:
```cpp
// Cookie utilities
std::map<std::string, std::string> parseCookieHeader(const std::string& cookieHeader);
std::string formatSetCookie(const std::string& name, const std::string& value,
                           const std::string& path = "/",
                           int maxAge = -1,
                           bool httpOnly = false,
                           bool secure = false);
```

**File:** `src/models/srcs/HttpUtils.cpp`

Implement cookie parsing:
```cpp
std::map<std::string, std::string> parseCookieHeader(const std::string& cookieHeader) {
    std::map<std::string, std::string> cookies;
    
    // Format: "name1=value1; name2=value2; name3=value3"
    std::string::size_type pos = 0;
    
    while (pos < cookieHeader.length()) {
        // Skip whitespace
        while (pos < cookieHeader.length() && (cookieHeader[pos] == ' ' || cookieHeader[pos] == '\t'))
            pos++;
        
        // Find next '=' for name
        std::string::size_type eqPos = cookieHeader.find('=', pos);
        if (eqPos == std::string::npos)
            break;
        
        std::string name = cookieHeader.substr(pos, eqPos - pos);
        name = trim(name);
        
        // Find end of value (semicolon or end of string)
        pos = eqPos + 1;
        std::string::size_type semicolonPos = cookieHeader.find(';', pos);
        
        std::string value;
        if (semicolonPos == std::string::npos) {
            value = cookieHeader.substr(pos);
            pos = cookieHeader.length();
        } else {
            value = cookieHeader.substr(pos, semicolonPos - pos);
            pos = semicolonPos + 1;
        }
        
        value = trim(value);
        
        if (!name.empty())
            cookies[name] = value;
    }
    
    return cookies;
}

std::string formatSetCookie(const std::string& name, const std::string& value,
                           const std::string& path,
                           int maxAge,
                           bool httpOnly,
                           bool secure) {
    std::string setCookie = name + "=" + value;
    
    if (!path.empty())
        setCookie += "; Path=" + path;
    
    if (maxAge >= 0) {
        std::ostringstream oss;
        oss << maxAge;
        setCookie += "; Max-Age=" + oss.str();
    }
    
    if (httpOnly)
        setCookie += "; HttpOnly";
    
    if (secure)
        setCookie += "; Secure";
    
    // Add SameSite for CSRF protection
    setCookie += "; SameSite=Lax";
    
    return setCookie;
}
```

#### Step 3: Integrate Cookie Parsing into HttpParser

**File:** `src/models/srcs/HttpParser.cpp`

After parsing headers, extract cookies:
```cpp
// In parseRequest() after headers are parsed:

// Parse Cookie header if present
std::map<std::string, std::string>::const_iterator cookieIt = headers.find("cookie");
if (cookieIt != headers.end()) {
    std::map<std::string, std::string> cookies = parseCookieHeader(cookieIt->second);
    request->setCookies(cookies);  // Need to add this setter
}
```

**File:** `src/models/headers/HttpRequest.hpp`

Add setter:
```cpp
void setCookies(const std::map<std::string, std::string>& cookies);
```

**File:** `src/models/srcs/HttpRequest.cpp`

```cpp
void HttpRequest::setCookies(const std::map<std::string, std::string>& cookies) {
    _cookies = cookies;
}
```

#### Step 4: Pass Cookies to CGI Scripts

**File:** `src/models/srcs/CgiHandle.cpp`

In `buildCgiEnvironment()`, add cookies:
```cpp
void CgiHandle::buildCgiEnvironment(...) {
    // ... existing environment variables ...
    
    // Add HTTP_COOKIE environment variable
    const std::map<std::string, std::string>& cookies = request.getCookies();
    if (!cookies.empty()) {
        std::string cookieStr;
        for (std::map<std::string, std::string>::const_iterator it = cookies.begin();
             it != cookies.end(); ++it) {
            if (!cookieStr.empty())
                cookieStr += "; ";
            cookieStr += it->first + "=" + it->second;
        }
        envVars["HTTP_COOKIE"] = cookieStr;
    }
    
    // ... rest of function ...
}
```

#### Step 5: Support Set-Cookie in Responses

**File:** `src/models/headers/HttpResponse.hpp`

Add convenience method:
```cpp
void setCookie(const std::string& name, const std::string& value,
               const std::string& path = "/",
               int maxAge = -1,
               bool httpOnly = false,
               bool secure = false);
```

**File:** `src/models/srcs/HttpResponse.cpp`

```cpp
void HttpResponse::setCookie(const std::string& name, const std::string& value,
                            const std::string& path,
                            int maxAge,
                            bool httpOnly,
                            bool secure) {
    std::string cookieValue = formatSetCookie(name, value, path, maxAge, httpOnly, secure);
    
    // Note: Set-Cookie can appear multiple times, one per cookie
    // We need to handle this specially since headers map uses unique keys
    
    // Option 1: Store as array internally
    // Option 2: Append to existing with \r\n delimiter
    // Option 3: Store in separate cookies vector
    
    // For simplicity, we'll append multiple Set-Cookie headers during serialization
    setHeader("Set-Cookie", cookieValue);
}
```

**Note**: HTTP allows multiple `Set-Cookie` headers. The current header storage (`std::map`) might need adjustment to support this. Options:

1. **Store cookies separately**: Add `std::vector<std::string> _cookies` in HttpResponse
2. **Change headers to multimap**: `std::multimap<std::string, std::string> _headers`
3. **Join with special delimiter**: Store multiple Set-Cookie values and split during serialization

**Recommended**: Add separate cookie storage:

```cpp
// In HttpResponse.hpp
private:
    std::vector<std::string> _setCookies;  // Multiple Set-Cookie headers

public:
    void addSetCookie(const std::string& setCookieValue);
    const std::vector<std::string>& getSetCookies() const;
```

Then in `serialize()`:
```cpp
// After headers
for (size_t i = 0; i < _setCookies.size(); i++) {
    response += "Set-Cookie: " + _setCookies[i] + "\r\n";
}
```

#### Step 6: CGI Set-Cookie Support

CGI scripts can already set cookies via stdout:
```python
#!/usr/bin/env python3
print("Content-Type: text/html")
print("Set-Cookie: session_id=abc123; Path=/; HttpOnly")
print("")  # Blank line separates headers from body
print("<html><body>Cookie set!</body></html>")
```

The CGI handler needs to parse these headers properly.

**File:** `src/models/srcs/CgiHandle.cpp`

Update `sendCgiOutputToClient()` to handle Set-Cookie:
```cpp
void CgiHandle::sendCgiOutputToClient(const std::string &cgiOutput, HttpResponse &res) {
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = cgiOutput.find("\n\n");
    
    if (headerEnd != std::string::npos) {
        std::string headers = cgiOutput.substr(0, headerEnd);
        std::string body = cgiOutput.substr(headerEnd + 4);  // Skip \r\n\r\n or \n\n
        
        // Parse CGI headers
        std::istringstream headerStream(headers);
        std::string line;
        
        while (std::getline(headerStream, line)) {
            if (line.empty() || line == "\r")
                break;
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = trim(line.substr(0, colonPos));
                std::string headerValue = trim(line.substr(colonPos + 1));
                
                // Handle Set-Cookie specially (can have multiple)
                if (toLowerStr(headerName) == "set-cookie") {
                    res.addSetCookie(headerValue);
                } else {
                    res.setHeader(headerName, headerValue);
                }
            }
        }
        
        res.setBody(body);
    } else {
        // No headers, just body
        res.setBody(cgiOutput);
    }
}
```

### Cookie Security Best Practices

1. **HttpOnly Flag**: Prevents JavaScript access (XSS protection)
   ```cpp
   res.setCookie("session_id", "abc123", "/", 3600, true, false);
   ```

2. **Secure Flag**: Only send over HTTPS
   ```cpp
   res.setCookie("token", "xyz", "/", -1, true, true);
   ```

3. **SameSite**: CSRF protection
   - `Strict`: Cookie only sent to same site
   - `Lax`: Cookie sent on top-level navigation
   - `None`: Cookie sent with all requests (requires Secure)

4. **Short Expiration**: Session cookies expire when browser closes
   ```cpp
   res.setCookie("temp", "value", "/", -1);  // Session cookie
   ```

5. **Path Restriction**: Limit cookie to specific paths
   ```cpp
   res.setCookie("admin_token", "abc", "/admin", 3600);
   ```

### Cookie Use Cases

#### Use Case 1: Session Management
```cpp
// Login handler
void handleLogin(HttpRequest& req, HttpResponse& res) {
    std::string username = req.getPostParam("username");
    std::string password = req.getPostParam("password");
    
    if (authenticate(username, password)) {
        std::string sessionId = generateSessionId();
        res.setCookie("session_id", sessionId, "/", 3600, true, false);
        res.setStatus(200, "OK");
        res.setBody("Login successful");
    } else {
        res.setStatus(401, "Unauthorized");
        res.setBody("Invalid credentials");
    }
}
```

#### Use Case 2: User Preferences
```cpp
// Theme handler
void handleTheme(HttpRequest& req, HttpResponse& res) {
    std::string theme = req.getQueryParam("theme");
    
    if (theme == "dark" || theme == "light") {
        res.setCookie("user_theme", theme, "/", 31536000, false, false);  // 1 year
        res.setStatus(200, "OK");
        res.setBody("Theme updated");
    }
}
```

#### Use Case 3: Reading Cookies
```cpp
// Check if user is logged in
void handleProfile(HttpRequest& req, HttpResponse& res) {
    if (req.hasCookie("session_id")) {
        std::string sessionId = req.getCookie("session_id");
        if (isValidSession(sessionId)) {
            res.setStatus(200, "OK");
            res.setBody(getUserProfile(sessionId));
        } else {
            res.setStatus(401, "Unauthorized");
            res.setBody("Invalid session");
        }
    } else {
        res.setStatus(401, "Unauthorized");
        res.setBody("Please login");
    }
}
```

---

## Implementation Roadmap

### Phase 1: Auto Index (Estimated: 4-6 hours)

1. ✅ **Planning** (You are here)
2. ⏳ **Add utility functions to HttpUtils**
   - `generateDirectoryListing()`
   - `formatFileSize()`
   - `formatTimestamp()`
   - Time: ~1 hour
3. ⏳ **Update HttpRequest.cpp**
   - Replace TODO with directory listing logic
   - Test with various directory structures
   - Time: ~1 hour
4. ⏳ **Testing**
   - Empty directories
   - Nested directories
   - Permissions (403 errors)
   - Large directories (100+ files)
   - Special characters in filenames
   - Time: ~2 hours

### Phase 2: Cookie Handling (Estimated: 6-8 hours)

1. ⏳ **Add cookie data structures**
   - Update `HttpRequest` with cookie storage
   - Add cookie accessors
   - Time: ~1 hour
2. ⏳ **Implement cookie parsing**
   - Add `parseCookieHeader()` to HttpUtils
   - Add `formatSetCookie()` helper
   - Integrate into HttpParser
   - Time: ~2 hours
3. ⏳ **Update HttpResponse**
   - Add `Set-Cookie` support
   - Handle multiple cookies
   - Implement `setCookie()` method
   - Time: ~1.5 hours
4. ⏳ **CGI Integration**
   - Pass cookies to CGI via `HTTP_COOKIE` env var
   - Parse `Set-Cookie` from CGI output
   - Time: ~1.5 hours
5. ⏳ **Testing**
   - Cookie parsing from requests
   - Multiple cookies
   - Cookie attributes (HttpOnly, Secure, etc.)
   - CGI cookie exchange
   - Browser testing
   - Time: ~2 hours

### Phase 3: Integration & Polish (Estimated: 2-3 hours)

1. ⏳ **Documentation**
   - Update README with cookie examples
   - Add examples to docs
2. ⏳ **Security review**
   - Validate cookie parsing
   - Check for injection vulnerabilities
   - Review HttpOnly/Secure implementation
3. ⏳ **Final testing**
   - End-to-end testing with real browsers
   - Session management example
   - Cookie security validation

**Total Estimated Time:** 12-17 hours

---

## File Structure

### New/Modified Files

```
src/models/
├── headers/
│   ├── HttpUtils.hpp          # ✏️ Add directory & cookie utility declarations
│   ├── HttpRequest.hpp        # ✏️ Add cookie storage and accessors
│   └── HttpResponse.hpp       # ✏️ Add setCookie method and Set-Cookie storage
├── srcs/
│   ├── HttpUtils.cpp          # ✏️ Implement directory listing & cookie utilities
│   ├── HttpRequest.cpp        # ✏️ Replace TODO, add cookie accessors
│   ├── HttpParser.cpp         # ✏️ Parse Cookie header
│   ├── HttpResponse.cpp       # ✏️ Implement setCookie and serialize Set-Cookie
│   └── CgiHandle.cpp          # ✏️ Pass HTTP_COOKIE env var, parse Set-Cookie from CGI

docs/
└── AUTOINDEX_AND_CACHE_PLAN.md  # ✅ This file (updated for cookies)

Tests/
├── autoindex_tests.sh         # 📝 New test script
└── cookie_tests.sh            # 📝 New test script

www/
└── cgi-bin/
    ├── test_cookie_set.py     # 📝 New CGI test - sets cookies
    ├── test_cookie_read.py    # 📝 New CGI test - reads cookies
    └── test_session.py        # 📝 New CGI test - session management
```

---

## Testing Strategy

### Auto Index Testing

Create test script: `Tests/autoindex_tests.sh`

```bash
#!/bin/bash

echo "=== Auto Index Tests ==="

# Test 1: Directory with files
echo "Test 1: Directory with files"
curl -i http://localhost:8080/test_full_dir/

# Test 2: Empty directory
echo "Test 2: Empty directory"
mkdir -p www/empty_dir
curl -i http://localhost:8080/empty_dir/

# Test 3: Nested directories
echo "Test 3: Nested directories"
mkdir -p www/nested/subdir/deep
curl -i http://localhost:8080/nested/

# Test 4: Directory without autoindex (should 403 or 404)
echo "Test 4: Directory without autoindex"
curl -i http://localhost:8080/

# Test 5: Special characters in filenames
echo "Test 5: Special characters in filenames"
touch "www/test_full_dir/file with spaces.txt"
touch "www/test_full_dir/file-with-dashes.txt"
curl -i http://localhost:8080/test_full_dir/

# Test 6: Large directory
echo "Test 6: Large directory (100+ files)"
mkdir -p www/large_dir
for i in {1..150}; do
    touch "www/large_dir/file_$i.txt"
done
curl -i http://localhost:8080/large_dir/
```

### Cookie Testing

Create test script: `Tests/cookie_tests.sh`

```bash
#!/bin/bash

echo "=== Cookie Tests ==="

# Test 1: Set cookie via Set-Cookie header
echo "Test 1: Server sets cookie"
curl -i http://localhost:8080/cgi-bin/test_cookie_set.py

# Test 2: Send cookie in request
echo "Test 2: Send cookie to server"
curl -i -H "Cookie: session_id=abc123" http://localhost:8080/cgi-bin/test_cookie_read.py

# Test 3: Multiple cookies
echo "Test 3: Multiple cookies"
curl -i -H "Cookie: session_id=abc123; user_pref=dark_mode; lang=en" \
    http://localhost:8080/cgi-bin/test_cookie_read.py

# Test 4: Cookie with attributes
echo "Test 4: Cookie with HttpOnly, Secure, Path"
curl -i http://localhost:8080/cgi-bin/test_cookie_set.py?secure=true

# Test 5: Session management
echo "Test 5: Session management"
# First request - get session cookie
COOKIE=$(curl -si http://localhost:8080/cgi-bin/test_session.py?action=login | \
         grep -i "Set-Cookie:" | sed 's/Set-Cookie: //' | cut -d';' -f1)

echo "Got cookie: $COOKIE"

# Second request - use session cookie
curl -i -H "Cookie: $COOKIE" http://localhost:8080/cgi-bin/test_session.py?action=profile

# Third request - logout
curl -i -H "Cookie: $COOKIE" http://localhost:8080/cgi-bin/test_session.py?action=logout

# Test 6: Cookie parsing edge cases
echo "Test 6: Cookie parsing edge cases"
curl -i -H "Cookie: a=1; b=2; c=3" http://localhost:8080/cgi-bin/test_cookie_read.py
curl -i -H "Cookie: empty=; with_space= value ; trim=test" http://localhost:8080/cgi-bin/test_cookie_read.py
```

### CGI Cookie Test Scripts

**File:** `www/cgi-bin/test_cookie_set.py`

```python
#!/usr/bin/env python3
import os
import cgi

print("Content-Type: text/html")
print("Set-Cookie: session_id=xyz789; Path=/; HttpOnly")
print("Set-Cookie: user_pref=dark_mode; Max-Age=31536000; Path=/")
print("")

print("<html><head><title>Cookie Set</title></head><body>")
print("<h1>Cookies Set Successfully</h1>")
print("<p>Two cookies have been set:</p>")
print("<ul>")
print("<li><b>session_id</b> = xyz789 (HttpOnly, session)</li>")
print("<li><b>user_pref</b> = dark_mode (1 year)</li>")
print("</ul>")
print("<p><a href='/cgi-bin/test_cookie_read.py'>Read cookies</a></p>")
print("</body></html>")
```

**File:** `www/cgi-bin/test_cookie_read.py`

```python
#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print("")

print("<html><head><title>Cookie Read</title></head><body>")
print("<h1>Cookies Received</h1>")

http_cookie = os.environ.get('HTTP_COOKIE', '')

if http_cookie:
    print("<p>Raw Cookie header: <code>" + http_cookie + "</code></p>")
    print("<h2>Parsed Cookies:</h2>")
    print("<table border='1' cellpadding='5'>")
    print("<tr><th>Name</th><th>Value</th></tr>")
    
    cookies = [c.strip().split('=', 1) for c in http_cookie.split(';')]
    for cookie in cookies:
        if len(cookie) == 2:
            name, value = cookie
            print(f"<tr><td><b>{name}</b></td><td>{value}</td></tr>")
    
    print("</table>")
else:
    print("<p><i>No cookies received</i></p>")

print("<p><a href='/cgi-bin/test_cookie_set.py'>Set cookies</a></p>")
print("</body></html>")
```

**File:** `www/cgi-bin/test_session.py`

```python
#!/usr/bin/env python3
import os
import cgi
import random
import string

def generate_session_id():
    return ''.join(random.choices(string.ascii_letters + string.digits, k=32))

print("Content-Type: text/html")

# Parse query string
query = os.environ.get('QUERY_STRING', '')
params = dict(p.split('=') for p in query.split('&') if '=' in p)
action = params.get('action', 'index')

# Get current session cookie
http_cookie = os.environ.get('HTTP_COOKIE', '')
session_id = None
if http_cookie:
    cookies = dict(c.strip().split('=', 1) for c in http_cookie.split(';') if '=' in c)
    session_id = cookies.get('session_id')

if action == 'login':
    # Set new session
    new_session = generate_session_id()
    print(f"Set-Cookie: session_id={new_session}; Path=/; HttpOnly; Max-Age=3600")
    print("")
    print("<html><body>")
    print(f"<h1>Login Successful</h1>")
    print(f"<p>Session ID: {new_session}</p>")
    print("<p><a href='/cgi-bin/test_session.py?action=profile'>View Profile</a></p>")
    print("</body></html>")

elif action == 'profile':
    print("")
    print("<html><body>")
    if session_id:
        print("<h1>Profile Page</h1>")
        print(f"<p>Logged in with session: {session_id}</p>")
        print("<p><a href='/cgi-bin/test_session.py?action=logout'>Logout</a></p>")
    else:
        print("<h1>Not Logged In</h1>")
        print("<p><a href='/cgi-bin/test_session.py?action=login'>Login</a></p>")
    print("</body></html>")

elif action == 'logout':
    # Delete session cookie
    print("Set-Cookie: session_id=; Path=/; Max-Age=0")
    print("")
    print("<html><body>")
    print("<h1>Logged Out</h1>")
    print("<p><a href='/cgi-bin/test_session.py?action=login'>Login Again</a></p>")
    print("</body></html>")

else:
    print("")
    print("<html><body>")
    print("<h1>Session Test</h1>")
    if session_id:
        print(f"<p>Current session: {session_id}</p>")
        print("<p><a href='/cgi-bin/test_session.py?action=profile'>Profile</a> | ")
        print("<a href='/cgi-bin/test_session.py?action=logout'>Logout</a></p>")
    else:
        print("<p>Not logged in</p>")
        print("<p><a href='/cgi-bin/test_session.py?action=login'>Login</a></p>")
    print("</body></html>")
```

### Browser Testing

1. **Chrome DevTools**:
   - Open Network tab
   - Check Request Headers for `Cookie:`
   - Check Response Headers for `Set-Cookie:`
   - Use Application → Cookies to view stored cookies
   - Verify HttpOnly cookies can't be accessed via JavaScript

2. **Firefox**:
   - Use Developer Tools → Storage → Cookies
   - Inspect cookie attributes (Domain, Path, Expires, HttpOnly, Secure)
   - Test cookie deletion

3. **Manual Testing**:
   - Visit `/cgi-bin/test_cookie_set.py`
   - Refresh page - cookies should be sent back
   - Visit `/cgi-bin/test_cookie_read.py` - should display cookies
   - Test session flow: login → profile → logout
   - Clear cookies and verify logout works

---

## Additional Considerations

### Security

1. **Directory Traversal Prevention**:
   - Already handled by existing path validation
   - Ensure `generateDirectoryListing()` doesn't follow symlinks outside root

2. **Information Disclosure**:
   - Auto index reveals directory structure
   - Only enable when explicitly configured
   - Consider adding `.hidden` file support to hide sensitive files

3. **Cookie Security**:
   - **HttpOnly**: Prevents XSS attacks from stealing cookies
   - **Secure**: Ensures cookies only sent over HTTPS (important for production)
   - **SameSite**: Prevents CSRF attacks
   - **Path/Domain**: Limit cookie scope to prevent leakage
   - **Validation**: Sanitize cookie values to prevent injection attacks

### Performance

1. **Directory Listing**:
   - For very large directories (1000+ files), consider pagination
   - Cache generated HTML for frequently accessed directories

2. **Cookie Handling**:
   - Minimal overhead (just string parsing)
   - Cookie storage is per-request (no server-side storage)
   - Consider limiting total cookie size (4KB per domain is browser limit)

### Future Enhancements

1. **Auto Index**:
   - Icon support for different file types
   - Sorting options (by name, size, date)
   - Search/filter functionality
   - JSON format option for APIs
   - Thumbnail support for images

2. **Cookies**:
   - Session storage (server-side session management)
   - Cookie encryption
   - Cookie signing for tamper detection
   - Cookie prefixes (`__Secure-`, `__Host-`)
   - Configuration for default cookie attributes

### RFC References

1. **Auto Index**:
   - No specific RFC (implementation-specific)
   - Follow common conventions (Apache, Nginx)

2. **Cookies**:
   - **RFC 6265**: HTTP State Management Mechanism (primary reference)
   - **RFC 2109**: HTTP State Management Mechanism (obsolete, but historical)
   - **RFC 2965**: HTTP State Management Mechanism (obsolete)
   - Key sections:
     - Section 4.1: Set-Cookie syntax
     - Section 4.2: Cookie syntax
     - Section 5.1.3: Cookie security

---

## Conclusion

This plan provides a clear roadmap for implementing both auto index and cookie handling. These features are essential for modern web applications:

- **Auto Index**: Improves usability when browsing directories
- **Cookies**: Enables session management, user preferences, and stateful interactions

**Recommended Order:**
1. Start with **Auto Index** (simpler, standalone feature)
2. Then implement **Cookie Handling** (more complex, requires careful testing)

**Priority**: 
- **Auto Index**: Nice-to-have, improves UX
- **Cookies**: Essential for CGI applications requiring state (login systems, shopping carts, etc.)

Both features integrate well with the existing CGI implementation and will make your web server more complete and production-ready.
