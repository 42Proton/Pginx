# Pginx - TODO List

**Project:** HTTP Web Server (C++98)  
**Repository:** 42Proton/Pginx  
**Branch:** socket-setup  
**Last Updated:** November 12, 2025

---

## 🔴 HIGH PRIORITY (Critical for Core Functionality)

### 1. Implement DELETE Method
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp`
- **Description:** DELETE is declared and mentioned in config but not implemented
- **Tasks:**
  - [ ] Uncomment DELETE in `makeRequestByMethod()` factory function
  - [ ] Implement `DeleteRequest::handle()` method
  - [ ] Add file/directory deletion logic
  - [ ] Handle permission errors (403)
  - [ ] Handle not found errors (404)
  - [ ] Add security checks (path traversal prevention)
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`
  - `src/models/headers/HttpRequest.hpp`

### 2. Complete Directory Listing (Auto-Index)
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp:196`
- **Description:** TODO comment - generate directory listing HTML when autoindex is on
- **Tasks:**
  - [ ] Create HTML generation function for directory listings
  - [ ] List files and directories with proper formatting
  - [ ] Add file sizes and modification dates
  - [ ] Add parent directory (..) navigation
  - [ ] Style with basic CSS
  - [ ] Handle empty directories
  - [ ] Sort entries (directories first, then files alphabetically)
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`
  - `src/models/headers/HttpUtils.hpp` (add utility functions)
  - `src/models/srcs/HttpUtils.cpp`

### 3. Parse `allow_methods` Directive
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/parser.cpp`, `src/models/srcs/lexer.cpp`
- **Description:** Config files use `allow_methods` but it's not parsed
- **Tasks:**
  - [ ] Add `allow_methods` to lexer's `isAttribute()` function
  - [ ] Implement parsing logic in `parseLocationDirective()`
  - [ ] Clear default methods before setting custom ones
  - [ ] Update `LocationConfig::setMethods()` to replace, not append
  - [ ] Test with various method combinations
- **Files to modify:**
  - `src/models/srcs/lexer.cpp`
  - `src/models/srcs/parser.cpp`
  - `src/models/srcs/LocationConfig.cpp`

### 4. Use Configured `client_max_body_size`
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/SocketManager.cpp`
- **Description:** Hardcoded MAX_BODY_SIZE instead of using config value
- **Tasks:**
  - [ ] Remove hardcoded `MAX_BODY_SIZE` (or make it a fallback max)
  - [ ] Get `client_max_body_size` from RequestContext during validation
  - [ ] Update `isBodyTooLarge()` to use configured value
  - [ ] Handle location-specific vs server-specific limits
  - [ ] Return 413 with proper error page when exceeded
- **Files to modify:**
  - `src/models/headers/SocketManager.hpp`
  - `src/models/srcs/SocketManager.cpp`

---

## 🟡 MEDIUM PRIORITY (Important for Production Use)

### 5. Implement Chunked Transfer Encoding
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/SocketManager.cpp`
- **Description:** Can detect chunked encoding but cannot process it
- **Tasks:**
  - [ ] Implement chunk parser (hex size + data + CRLR)
  - [ ] Handle chunk extensions
  - [ ] Detect and process final chunk (0\r\n\r\n)
  - [ ] Assemble chunks into complete body
  - [ ] Handle trailer headers
  - [ ] Add error handling for malformed chunks
- **Files to modify:**
  - `src/models/srcs/SocketManager.cpp`
  - `src/models/srcs/HttpParser.cpp`

### 6. Implement Keep-Alive Connections
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/SocketManager.cpp`
- **Description:** Currently closes connection after each request
- **Tasks:**
  - [ ] Parse `Connection` header (keep-alive / close)
  - [ ] Implement connection reuse logic
  - [ ] Track connections and their states
  - [ ] Handle HTTP/1.0 vs HTTP/1.1 defaults
  - [ ] Implement max requests per connection limit
  - [ ] Add configurable keepalive timeout
  - [ ] Clean up idle kept-alive connections
- **Files to modify:**
  - `src/models/srcs/SocketManager.cpp`
  - `src/models/headers/SocketManager.hpp`

### 7. Implement Virtual Host (Server Name) Matching
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/SocketManager.cpp`
- **Description:** `selectServerForClient()` doesn't match Host header
- **Tasks:**
  - [ ] Extract `Host` header from request
  - [ ] Match against `server_name` directives
  - [ ] Handle exact matches
  - [ ] Handle wildcard server names (*.example.com)
  - [ ] Implement default server selection fallback
  - [ ] Handle port in Host header
- **Files to modify:**
  - `src/models/srcs/SocketManager.cpp`

### 8. Implement Redirect/Return Directive
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/parser.cpp`, `BaseBlock`
- **Description:** Mentioned in lexer but not parsed or handled
- **Tasks:**
  - [ ] Parse `return` directive in config (status code + URL)
  - [ ] Store in `BaseBlock::_returnData`
  - [ ] Check for return directive during request handling
  - [ ] Generate redirect response (301, 302, 307, 308)
  - [ ] Support both location and server level returns
  - [ ] Handle relative vs absolute URLs
- **Files to modify:**
  - `src/models/srcs/parser.cpp`
  - `src/models/srcs/HttpRequest.cpp`
  - `src/models/srcs/BaseBlock.cpp`

### 9. Implement PUT Method
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp`
- **Description:** Declared but not implemented
- **Tasks:**
  - [ ] Uncomment PUT in `makeRequestByMethod()`
  - [ ] Implement `PutRequest::handle()` method
  - [ ] Create or overwrite file at specified path
  - [ ] Handle directory creation if needed
  - [ ] Return 201 (Created) or 200 (OK)
  - [ ] Add security checks
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`

### 10. Implement PATCH Method
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp`
- **Description:** Declared but not implemented
- **Tasks:**
  - [ ] Uncomment PATCH in `makeRequestByMethod()`
  - [ ] Implement `PatchRequest::handle()` method
  - [ ] Parse patch format (JSON Patch, etc.)
  - [ ] Apply partial modifications
  - [ ] Return appropriate status codes
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`

---

## 🟢 LOW PRIORITY (Nice to Have)

### 11. Implement CGI Support
- **Status:** ❌ Not Started
- **Location:** New files needed
- **Description:** Execute external scripts (PHP, Python, etc.)
- **Tasks:**
  - [ ] Parse `cgi` directive from config
  - [ ] Detect CGI scripts by extension
  - [ ] Fork process for script execution
  - [ ] Set up environment variables (PATH_INFO, QUERY_STRING, etc.)
  - [ ] Pipe request body to script STDIN
  - [ ] Read script STDOUT as response
  - [ ] Parse CGI headers from output
  - [ ] Handle script timeouts
  - [ ] Handle script errors
- **Files to create/modify:**
  - `src/models/headers/CgiHandler.hpp`
  - `src/models/srcs/CgiHandler.cpp`
  - `src/models/srcs/parser.cpp`

### 12. Expand MIME Type Support
- **Status:** ⚠️ Partial (6 types only)
- **Location:** `src/utils.cpp:91-103`
- **Description:** Only supports 6 MIME types
- **Tasks:**
  - [ ] Add common text formats (XML, CSV, TXT, MD)
  - [ ] Add document formats (PDF, DOC, DOCX)
  - [ ] Add archive formats (ZIP, TAR, GZ)
  - [ ] Add video formats (MP4, WEBM, OGG)
  - [ ] Add audio formats (MP3, WAV, OGG)
  - [ ] Add font formats (WOFF, WOFF2, TTF, OTF)
  - [ ] Consider using MIME type database or map
- **Files to modify:**
  - `src/utils.cpp`

### 13. Implement Range Requests (Partial Content)
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp`
- **Description:** Support byte-range requests for large files
- **Tasks:**
  - [ ] Parse `Range` header
  - [ ] Support single byte range
  - [ ] Support multiple byte ranges (multipart/byteranges)
  - [ ] Return 206 Partial Content
  - [ ] Add `Content-Range` header
  - [ ] Add `Accept-Ranges: bytes` header
  - [ ] Handle invalid ranges (416)
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`
  - `src/models/srcs/HttpResponse.cpp`

### 14. Implement Response Compression
- **Status:** ❌ Not Started
- **Location:** New functionality
- **Description:** Compress responses with gzip/deflate
- **Tasks:**
  - [ ] Parse `Accept-Encoding` header
  - [ ] Implement gzip compression
  - [ ] Implement deflate compression
  - [ ] Add `Content-Encoding` header
  - [ ] Add `Vary: Accept-Encoding` header
  - [ ] Make compression optional per location/file type
  - [ ] Set minimum size threshold for compression
- **Dependencies:** May need zlib library
- **Files to create/modify:**
  - `src/models/headers/CompressionUtils.hpp`
  - `src/models/srcs/CompressionUtils.cpp`
  - `src/models/srcs/HttpResponse.cpp`

### 15. Improve Access Logging
- **Status:** ⚠️ Minimal (stderr only)
- **Location:** Throughout codebase
- **Description:** Add proper access and error logs
- **Tasks:**
  - [ ] Implement access log format (combined/common)
  - [ ] Log each request with timestamp, method, path, status, size
  - [ ] Implement error log with levels (ERROR, WARN, INFO, DEBUG)
  - [ ] Make log paths configurable
  - [ ] Add log file rotation support
  - [ ] Add option to log to stdout/stderr or file
  - [ ] Parse `access_log` and `error_log` directives
- **Files to create/modify:**
  - `src/models/headers/Logger.hpp`
  - `src/models/srcs/Logger.cpp`
  - `src/models/srcs/parser.cpp`

### 16. Add Security Headers
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpResponse.cpp`
- **Description:** Add standard security headers to responses
- **Tasks:**
  - [ ] Add `X-Content-Type-Options: nosniff`
  - [ ] Add `X-Frame-Options: DENY` (or configurable)
  - [ ] Add `X-XSS-Protection: 1; mode=block`
  - [ ] Add `Strict-Transport-Security` for HTTPS
  - [ ] Make headers configurable in config file
  - [ ] Add `add_header` directive parsing
- **Files to modify:**
  - `src/models/srcs/HttpResponse.cpp`
  - `src/models/srcs/parser.cpp`

### 17. Improve Location Matching
- **Status:** ⚠️ Basic only
- **Location:** `src/models/srcs/Server.cpp`
- **Description:** Only exact/prefix matching, no regex
- **Tasks:**
  - [ ] Implement regex location matching `location ~ pattern {}`
  - [ ] Implement case-insensitive regex `location ~* pattern {}`
  - [ ] Implement location priority (exact > regex > prefix)
  - [ ] Add `^~` modifier for priority prefix matching
  - [ ] Add `=` modifier for exact matching
  - [ ] Update `findLocation()` with proper matching logic
- **Dependencies:** May need regex library (C++11 or POSIX regex)
- **Files to modify:**
  - `src/models/srcs/Server.cpp`
  - `src/models/headers/Server.hpp`
  - `src/models/srcs/parser.cpp`

### 18. Multipart Form Data Parsing
- **Status:** ❌ Not Started
- **Location:** `src/models/srcs/HttpRequest.cpp`
- **Description:** Support file uploads with proper multipart parsing
- **Tasks:**
  - [ ] Detect `Content-Type: multipart/form-data`
  - [ ] Extract boundary from Content-Type header
  - [ ] Parse multipart sections
  - [ ] Extract form fields and files
  - [ ] Save uploaded files to upload directory
  - [ ] Generate unique filenames for uploads
  - [ ] Handle multiple file uploads
  - [ ] Implement size limits per file
- **Files to modify:**
  - `src/models/srcs/HttpRequest.cpp`
  - `src/models/headers/HttpUtils.hpp`
  - `src/models/srcs/HttpUtils.cpp`

---

## 🔧 CODE QUALITY & REFACTORING

### 19. Memory Management Improvements
- **Status:** ⚠️ Needs review
- **Location:** Throughout codebase
- **Tasks:**
  - [ ] Review all `new` allocations for matching `delete`
  - [ ] Add RAII wrappers where appropriate
  - [ ] Ensure exception safety (no leaks on exceptions)
  - [ ] Review `HttpRequest*` pointer lifecycle
  - [ ] Consider smart pointers (if C++98 allows auto_ptr)

### 20. Error Handling Consistency
- **Status:** ⚠️ Inconsistent
- **Location:** Throughout codebase
- **Tasks:**
  - [ ] Standardize error handling strategy
  - [ ] Document which functions throw vs return errors
  - [ ] Add error context information
  - [ ] Create custom exception hierarchy
  - [ ] Add error recovery mechanisms

### 21. Configuration Consolidation
- **Status:** ⚠️ Magic numbers present
- **Location:** Throughout codebase
- **Tasks:**
  - [ ] Move all hardcoded limits to config or constants
  - [ ] Make timeouts configurable
  - [ ] Make buffer sizes configurable
  - [ ] Document all configurable options
  - [ ] Add config validation

### 22. Request Validation Improvements
- **Status:** ⚠️ Basic validation only
- **Location:** `src/models/srcs/SocketManager.cpp`
- **Tasks:**
  - [ ] Validate header field names (no invalid characters)
  - [ ] Check for duplicate Content-Length headers
  - [ ] Validate Content-Length matches actual body size
  - [ ] Validate request target format
  - [ ] Add stricter HTTP version validation
  - [ ] Validate header value format

### 23. Add Unit Tests
- **Status:** ⚠️ Integration tests only
- **Location:** `Tests/` directory
- **Tasks:**
  - [ ] Add unit tests for parser
  - [ ] Add unit tests for HTTP request/response
  - [ ] Add unit tests for utility functions
  - [ ] Add unit tests for configuration classes
  - [ ] Set up test framework (if needed)
  - [ ] Add CI/CD testing

---

## 📚 DOCUMENTATION

### 24. Code Documentation
- **Status:** ⚠️ Minimal
- **Tasks:**
  - [ ] Add Doxygen-style comments to all classes
  - [ ] Document all public methods
  - [ ] Document configuration file format
  - [ ] Add architecture documentation
  - [ ] Create developer guide

### 25. User Documentation
- **Status:** ❌ Not Started
- **Tasks:**
  - [ ] Create README.md with usage instructions
  - [ ] Document configuration directives
  - [ ] Add example configurations
  - [ ] Create troubleshooting guide
  - [ ] Add performance tuning guide

---

## 🐛 KNOWN BUGS & ISSUES

### 26. Connection Close After Single Request
- **Status:** 🐛 Bug (by design currently)
- **Description:** Server closes connection after every request
- **Fix:** Implement Keep-Alive (see #6)

### 27. Server Selection Ignores Host Header
- **Status:** 🐛 Bug
- **Description:** Always selects first server regardless of Host header
- **Fix:** Implement proper matching (see #7)

### 28. Config Body Size Limit Ignored
- **Status:** 🐛 Bug
- **Description:** Uses hardcoded limit instead of configured value
- **Fix:** See #4

---

## 📊 TRACKING

### Statistics
- **Total Tasks:** 28
- **High Priority:** 4
- **Medium Priority:** 6
- **Low Priority:** 10
- **Code Quality:** 5
- **Documentation:** 2
- **Bugs:** 3

### Completion Status
- ❌ Not Started: 23
- ⚠️ Partial: 5
- ✅ Complete: 0

---

## 🎯 SUGGESTED IMPLEMENTATION ORDER

### Phase 1: Core Functionality (Week 1-2)
1. Parse `allow_methods` directive (#3)
2. Use configured `client_max_body_size` (#4)
3. Implement DELETE method (#1)
4. Complete directory listing (#2)

### Phase 2: Connection Management (Week 3)
5. Virtual host matching (#7)
6. Keep-Alive connections (#6)
7. Chunked transfer encoding (#5)

### Phase 3: Enhanced Features (Week 4-5)
8. PUT method (#9)
9. Redirect/return directive (#8)
10. Multipart form data (#18)
11. Expand MIME types (#12)

### Phase 4: Advanced Features (Week 6+)
12. CGI support (#11)
13. Range requests (#13)
14. Response compression (#14)
15. Improved logging (#15)

### Phase 5: Polish (Ongoing)
16. Security headers (#16)
17. Code quality improvements (#19-23)
18. Documentation (#24-25)

---

## 📝 NOTES

- All tasks should maintain C++98 compatibility
- Test thoroughly after each implementation
- Update this file as tasks are completed
- Add new issues as they're discovered
- Consider performance implications for each feature

---

**Last Review:** November 12, 2025  
**Next Review:** [To be scheduled]
