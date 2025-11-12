# HTTP Request Implementation - Weaknesses & Missing Features

**File:** `src/models/srcs/HttpRequest.cpp`  
**Analysis Date:** November 12, 2025  
**Branch:** http_last_methods  

---

## 📋 Executive Summary

This document identifies **critical bugs**, **missing implementations**, and **security weaknesses** in the HTTP request handling system. The codebase currently implements GET, HEAD, and POST methods, but DELETE, PUT, and PATCH are declared in headers but **not implemented**. Additionally, several logic bugs and security vulnerabilities exist in the current implementation.

---

## 🔴 CRITICAL MISSING IMPLEMENTATIONS

### 1. DELETE Method - Completely Missing

**Status:** ❌ **DECLARED BUT NOT IMPLEMENTED**

**Header Declaration:** `src/models/headers/HttpRequest.hpp` (lines 105-113)
```cpp
class DeleteRequest : public HttpRequest {
  public:
    DeleteRequest(const RequestContext &ctx);  // ✅ Correct signature
    virtual ~DeleteRequest();
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res);
};
```

**Implementation:** **NONE** - Completely missing from `HttpRequest.cpp`

**Factory Status:** Commented out (line 173)
```cpp
// if (method == "delete")
//     return new DeleteRequest();  // ❌ Wrong - missing ctx parameter!
```

**Impact:**
- DELETE requests will return NULL from factory
- Server will respond with 400 Bad Request for all DELETE requests
- Config files mention DELETE in `allow_methods` but it won't work

**Required Implementation:**
- Constructor: `DeleteRequest::DeleteRequest(const RequestContext &ctx)`
- Destructor: `DeleteRequest::~DeleteRequest()`
- Validation: Check for body (DELETE shouldn't have one)
- Handler: Check permissions, resolve path, verify existence, delete file/directory, return 204

---

### 2. PUT Method - Completely Missing

**Status:** ❌ **DECLARED WITH WRONG SIGNATURE**

**Header Declaration:** `src/models/headers/HttpRequest.hpp` (lines 97-103)
```cpp
class PutRequest : public HttpRequest {
  public:
    PutRequest();  // ❌ WRONG - Missing RequestContext parameter!
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res);
};
```

**Implementation:** **NONE** - Completely missing from `HttpRequest.cpp`

**Factory Status:** Commented out with incorrect signature (line 171)
```cpp
// if (method == "put")      // ❌ lowercase instead of uppercase
//     return new PutRequest();  // ❌ Can't compile - base class requires ctx
```

**Issues:**
1. Constructor signature doesn't match base class requirement
2. No implementation exists
3. Factory uses lowercase "put" instead of "PUT"
4. Can't be instantiated due to base class constructor requirement

**Header Fix Needed:**
```cpp
PutRequest(const RequestContext &ctx);  // ✅ Correct signature
```

---

### 3. PATCH Method - Completely Missing

**Status:** ❌ **DECLARED WITH WRONG SIGNATURE**

**Header Declaration:** `src/models/headers/HttpRequest.hpp` (lines 99-104)
```cpp
class PatchRequest : public HttpRequest {
  public:
    PatchRequest();  // ❌ WRONG - Missing RequestContext parameter!
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res);
};
```

**Implementation:** **NONE** - Completely missing from `HttpRequest.cpp`

**Factory Status:** Commented out with incorrect signature (line 172)
```cpp
// if (method == "patch")    // ❌ lowercase instead of uppercase
//     return new PatchRequest();  // ❌ Can't compile - base class requires ctx
```

**Issues:** Same as PUT method above

---

## 🐛 CRITICAL BUGS IN EXISTING CODE

### Bug 1: GET/HEAD Permission Check Logic Error

**Location:** `src/models/srcs/HttpRequest.cpp` lines 199-203

**Severity:** 🔴 **CRITICAL** - Security vulnerability

**Current Code:**
```cpp
if (!_ctx.isMethodAllowed("GET") && !_ctx.isMethodAllowed("HEAD"))
{
    res.setErrorFromContext(405, _ctx);
    return;
}
```

**Problem:** Uses `&&` (AND) operator instead of checking the actual method!

**Behavior:**
| Config | GET Request | HEAD Request | Expected | Actual | Result |
|--------|-------------|--------------|----------|--------|--------|
| Only GET allowed | ✅ Allow | ❌ 405 | ✅ Allow, ❌ 405 | ✅ Allow, ✅ Allow | 🐛 BUG |
| Only HEAD allowed | ❌ 405 | ✅ Allow | ❌ 405, ✅ Allow | ✅ Allow, ✅ Allow | 🐛 BUG |
| Both allowed | ✅ Allow | ✅ Allow | ✅ Allow, ✅ Allow | ✅ Allow, ✅ Allow | ✅ OK |
| Neither allowed | ❌ 405 | ❌ 405 | ❌ 405, ❌ 405 | ❌ 405, ❌ 405 | ✅ OK |

**Impact:** Method restrictions in config are ignored! If you configure a location to only allow HEAD, GET requests will also work.

**Correct Fix:**
```cpp
// Option 1: Check the actual method being used
if (!_ctx.isMethodAllowed(method))
{
    res.setErrorFromContext(405, _ctx);
    return;
}

// Option 2: Explicit check for each method
if ((method == "GET" && !_ctx.isMethodAllowed("GET")) ||
    (method == "HEAD" && !_ctx.isMethodAllowed("HEAD")))
{
    res.setErrorFromContext(405, _ctx);
    return;
}
```

---

### Bug 2: POST Validation Comment vs Code Contradiction

**Location:** `src/models/srcs/HttpRequest.cpp` lines 272-277

**Severity:** 🟡 **MEDIUM** - Confusing behavior

**Current Code:**
```cpp
bool PostRequest::validate(std::string &err) const
{
    // POST can have empty body for some CGI scenarios
    if (contentLength() == 0)
    {
        err = "Missing body in POST request";
        return false;  // ❌ REJECTS empty body!
    }
    return true;
}
```

**Problem:** Comment says "POST can have empty body for CGI" but code **rejects** empty body by returning `false`!

**Impact:** 
- Contradictory documentation
- May break legitimate empty POST requests
- CGI scenarios mentioned in comment won't work

**Decision Needed:**
1. **Allow empty POST:** Remove the validation
2. **Disallow empty POST:** Remove the misleading comment

**Suggested Fix (if allowing empty):**
```cpp
bool PostRequest::validate(std::string &err) const
{
    // POST can have empty body (e.g., for CGI scenarios)
    // No validation needed - empty body is acceptable
    (void)err;
    return true;
}
```

---

### Bug 3: Factory Method Case Sensitivity

**Location:** `src/models/srcs/HttpRequest.cpp` lines 169-175

**Severity:** 🟢 **LOW** - Will cause issues when uncommented

**Current Code:**
```cpp
HttpRequest *makeRequestByMethod(const std::string &method, const RequestContext &ctx)
{
    if (method == "GET" || method == "HEAD")
        return new GetHeadRequest(ctx);
    if (method == "POST")
        return new PostRequest(ctx);
    // if (method == "put")      // ❌ Lowercase!
    // if (method == "patch")    // ❌ Lowercase!
    // if (method == "delete")   // ❌ Lowercase!
    return 0;
}
```

**Problem:** HTTP methods are **case-sensitive** and must be uppercase per RFC 7230. The commented code uses lowercase.

**Impact:** When uncommented, these checks will never match because HttpParser converts methods to uppercase.

**Correct Code:**
```cpp
if (method == "PUT")       // ✅ Uppercase
if (method == "PATCH")     // ✅ Uppercase
if (method == "DELETE")    // ✅ Uppercase
```

---

## ⚠️ SECURITY WEAKNESSES

### Weakness 1: Inadequate Path Traversal Protection

**Location:** `src/models/srcs/HttpRequest.cpp` line 289 (PostRequest)

**Severity:** 🔴 **CRITICAL** - Security vulnerability

**Current Code:**
```cpp
bool PostRequest::isPathSafe(const std::string &path) const
{
    if (path.find("..") != std::string::npos)
        return false;
    return true;
}
```

**Vulnerabilities:**

| Attack Vector | Example | Blocked? | Reason |
|---------------|---------|----------|--------|
| Simple traversal | `../../../etc/passwd` | ✅ Yes | Literal ".." detected |
| URL-encoded | `%2e%2e/etc/passwd` | ❌ No | Not decoded before check |
| Double-encoded | `%252e%252e/etc/passwd` | ❌ No | Not decoded |
| Absolute path | `/etc/passwd` | ❌ No | No absolute path check |
| Unicode | `\u002e\u002e/etc/passwd` | ❌ No | Unicode not handled |
| Null byte | `file.txt\0.pdf` | ❌ No | Null byte injection possible |

**Exploits:**
```bash
# These would bypass the current check:
curl -X POST http://server/upload/%2e%2e/%2e%2e/etc/passwd
curl -X POST http://server//etc/passwd
curl -X POST 'http://server/upload/..%2f..%2f/etc/passwd'
```

**Impact:**
- Arbitrary file write anywhere on system (if permissions allow)
- Overwrite critical system files
- Escalate privileges
- Data destruction

**Proper Solution:**
```cpp
bool PostRequest::isPathSafe(const std::string &fullPath) const
{
    // 1. Decode URL-encoded characters first
    std::string decodedPath = urlDecode(fullPath);
    
    // 2. Check for path traversal patterns
    if (decodedPath.find("..") != std::string::npos)
        return false;
    
    // 3. Check for absolute paths
    if (!decodedPath.empty() && decodedPath[0] == '/')
        return false;
    
    // 4. Canonicalize path and verify it starts with root directory
    // Use realpath() or similar to resolve symlinks and normalize
    
    // 5. Check for null bytes
    if (decodedPath.find('\0') != std::string::npos)
        return false;
    
    return true;
}
```

---

### Weakness 2: No Canonicalization of Paths

**Location:** Throughout request handlers

**Severity:** 🟡 **MEDIUM** - Bypasses security checks

**Problem:** Paths are not canonicalized (resolved to absolute form) before security checks.

**Attack Examples:**
```bash
# Multiple slashes
/uploads////file.txt

# Dot segments
/uploads/./././file.txt

# Mixed
/uploads/.//././../uploads/file.txt
```

**These all resolve to the same file but might bypass simple string checks.**

**Solution:** Use `realpath()` to canonicalize paths before validation.

---

## ⚠️ INCOMPLETE FEATURES

### Incomplete 1: Directory Listing (Auto-Index)

**Location:** `src/models/srcs/HttpRequest.cpp` lines 232-239

**Severity:** 🟡 **MEDIUM** - Feature doesn't work

**Current Code:**
```cpp
if (_ctx.getAutoIndex())
{
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", "text/html");
    if (includeBody)
    {
        // TODO: generate directory listing HTML
    }
    return;
}
```

**Problems:**
1. Returns 200 OK but with **empty body** (invalid HTTP response)
2. No `Content-Length` header set
3. No HTML generation for directory listing
4. Config option `autoindex on` is parsed but doesn't work

**Impact:**
- Browsers show blank page instead of directory listing
- Content-Length mismatch may cause connection issues
- Feature advertised in config doesn't work

**What's Needed:**
- Scan directory with `opendir()` / `readdir()`
- Generate HTML table with file names, sizes, dates
- Calculate and set `Content-Length`
- Handle sorting (directories first, then alphabetically)
- Add parent directory (..) link
- Handle permission errors gracefully

---

### Incomplete 2: No Content-Length for Directory Listing

**Location:** Same as above

**Problem:** When directory listing is generated (once implemented), the response lacks a `Content-Length` header.

**Impact:**
- HTTP/1.1 clients may hang waiting for more data
- Connection management issues
- Violates HTTP specification for responses with body

---

## ⚠️ VALIDATION GAPS

### Gap 1: POST Doesn't Verify Body Size Against Config

**Location:** `src/models/srcs/HttpRequest.cpp` POST handler

**Severity:** 🟢 **LOW** - Defense in depth

**Issue:** POST handler trusts that SocketManager already validated body size against `client_max_body_size`.

**Problem:** 
- No defensive validation in handler
- Possible race condition if limits change
- Violates principle of least privilege

**Recommendation:**
```cpp
void PostRequest::handle(HttpResponse &res)
{
    // Defensive validation
    if (body.size() > _ctx.getClientMaxBodySize()) {
        res.setErrorFromContext(413, _ctx);
        return;
    }
    
    // ... rest of handler
}
```

---

### Gap 2: No Upload Directory Existence Check

**Location:** POST handler before file write

**Issue:** Doesn't verify upload directory exists before attempting write.

**Current Behavior:**
- If upload dir doesn't exist, `std::ofstream` fails
- Returns 500 Internal Server Error
- Not user-friendly

**Better Behavior:**
- Check if upload directory exists
- Attempt to create it (if configured)
- Return 403 Forbidden or 500 with better error message

---

### Gap 3: No Filename Validation

**Location:** POST handler filename handling

**Issues:**
- No validation of extracted filename
- Could contain special characters
- Could be very long
- Could contain null bytes

**Attack Examples:**
```bash
# Path injection via filename
curl -X POST http://server/upload/../../../etc/passwd

# Null byte truncation
curl -X POST http://server/upload/malicious.php%00.txt
```

---

## 📊 SUMMARY TABLE

| Feature/Issue | Type | Severity | Status | Impact |
|---------------|------|----------|--------|--------|
| **DELETE implementation** | Missing | 🔴 Critical | ❌ Not implemented | Feature doesn't work |
| **PUT implementation** | Missing | 🟡 Medium | ❌ Not implemented | Feature doesn't work |
| **PATCH implementation** | Missing | 🟢 Low | ❌ Not implemented | Feature doesn't work |
| **Factory registration** | Missing | 🔴 Critical | ❌ Commented out | Methods return 400 |
| **GET/HEAD permission bug** | Bug | 🔴 Critical | 🐛 Incorrect logic | Security bypass |
| **POST validation mismatch** | Bug | 🟡 Medium | 🐛 Contradictory | Confusing behavior |
| **Factory case sensitivity** | Bug | 🟢 Low | 🐛 Wrong case | Won't work when enabled |
| **Path traversal check** | Security | 🔴 Critical | ⚠️ Weak | Arbitrary file write |
| **No path canonicalization** | Security | 🟡 Medium | ⚠️ Missing | Security bypass |
| **Directory listing** | Feature | 🟡 Medium | ⚠️ Incomplete | Returns empty body |
| **No Content-Length** | Feature | 🟢 Low | ⚠️ Missing | HTTP violation |
| **No body size check** | Validation | 🟢 Low | ⚠️ Missing | Defense in depth |
| **No directory check** | Validation | 🟢 Low | ⚠️ Missing | Poor error messages |
| **No filename validation** | Security | 🟡 Medium | ⚠️ Missing | Injection possible |

---

## 🎯 PRIORITY ACTION ITEMS

### Phase 1: Critical Security & Bugs (IMMEDIATE)

1. **Fix GET/HEAD permission check** (Bug 1)
   - Change `&&` to check actual method
   - Test with method-restricted locations
   - **Estimated Time:** 15 minutes

2. **Implement DELETE method** (Critical Missing)
   - Write constructor, destructor, validate, handle
   - Uncomment and fix factory registration
   - Test with curl DELETE requests
   - **Estimated Time:** 1-2 hours

3. **Improve path traversal protection** (Security Weakness 1)
   - Add URL decode before check
   - Add absolute path check
   - Add null byte check
   - Consider path canonicalization
   - **Estimated Time:** 1 hour

### Phase 2: Core Features (HIGH PRIORITY)

4. **Fix PUT/PATCH constructor signatures** (Header files)
   - Update both headers to take `RequestContext&`
   - **Estimated Time:** 5 minutes

5. **Implement PUT method** (Missing Implementation)
   - Similar to POST but overwrites existing files
   - **Estimated Time:** 1 hour

6. **Implement directory listing** (Incomplete Feature 1)
   - Scan directory
   - Generate HTML
   - Set Content-Length
   - **Estimated Time:** 2-3 hours

### Phase 3: Improvements (MEDIUM PRIORITY)

7. **Fix POST validation comment** (Bug 2)
   - Decide on empty body policy
   - Fix code or comment
   - **Estimated Time:** 10 minutes

8. **Add defensive body size validation** (Validation Gap 1)
   - Check body size in POST handler
   - **Estimated Time:** 10 minutes

9. **Add upload directory checks** (Validation Gap 2)
   - Verify directory exists
   - Better error messages
   - **Estimated Time:** 30 minutes

10. **Add filename validation** (Validation Gap 3)
    - Sanitize filenames
    - Check for injection
    - **Estimated Time:** 30 minutes

### Phase 4: Complete Implementation (LOW PRIORITY)

11. **Implement PATCH method** (Missing Implementation)
    - Research PATCH semantics
    - Implement partial updates
    - **Estimated Time:** 2-3 hours

12. **Add path canonicalization** (Security Weakness 2)
    - Use realpath() or similar
    - Verify paths stay within root
    - **Estimated Time:** 1 hour

---

## 🔍 TESTING RECOMMENDATIONS

### Critical Security Tests

```bash
# Test path traversal protection
curl -X POST http://localhost:8002/upload/%2e%2e/%2e%2e/etc/passwd -d "test"
curl -X POST http://localhost:8002//etc/passwd -d "test"

# Test method permission enforcement
# (configure location to only allow GET)
curl -X HEAD http://localhost:8002/test  # Should return 405

# Test DELETE (once implemented)
curl -X DELETE http://localhost:8002/uploads/test.txt
curl -X DELETE http://localhost:8002/../etc/passwd  # Should return 403
```

### Functionality Tests

```bash
# Test directory listing
curl http://localhost:8002/uploads/  # With autoindex on

# Test DELETE method
echo "test" > www/uploads/test.txt
curl -X DELETE http://localhost:8002/uploads/test.txt
ls www/uploads/  # Should not contain test.txt

# Test PUT method (once implemented)
curl -X PUT http://localhost:8002/uploads/new.txt -d "content"
```

---

## 📚 REFERENCES

- **RFC 7230:** HTTP/1.1 Message Syntax and Routing
- **RFC 7231:** HTTP/1.1 Semantics and Content
- **OWASP Top 10:** Path Traversal vulnerabilities
- **CWE-22:** Improper Limitation of a Pathname to a Restricted Directory

---

**Document Maintainer:** Development Team  
**Last Updated:** November 12, 2025  
**Next Review:** After implementing DELETE method  

---

## 📝 CHANGE LOG

| Date | Change | Author |
|------|--------|--------|
| 2025-11-12 | Initial analysis and documentation | Team |
| TBD | After DELETE implementation | TBD |
| TBD | After security improvements | TBD |
