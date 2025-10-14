# Listen Directive Parser Enhancement

## Changes Made

The parser now supports the `listen` directive in two formats:

### Supported Formats

1. **Port only**: `listen 8080;`

   - Uses default address `0.0.0.0`
   - Port: as specified

2. **Address:Port**: `listen 127.0.0.1:3000;`
   - Address: as specified (e.g., 127.0.0.1, 0.0.0.0, etc.)
   - Port: as specified

### Example Configuration

```nginx
http {
    server {
        listen 127.0.0.1:3000;  # Listen on localhost, port 3000
        server_name localhost;
        root pages/;
    }

    server {
        listen 8080;             # Listen on all interfaces (0.0.0.0), port 8080
        server_name example.com;
        root pages/;
    }

    server {
        listen 0.0.0.0:9090;    # Explicitly listen on all interfaces, port 9090
        server_name test.local;
        root pages/;
    }
}
```

## Modified Files

### 1. `src/models/srcs/parser.cpp`

**Function**: `parseBasicServerDirective()`

Added logic to detect and parse `address:port` format:

- Searches for `:` in the listen value
- If found: splits into address and port
- If not found: treats as port only with default address `0.0.0.0`
- Calls `server.insertListen(port, addr)` with both parameters

### 2. `src/models/srcs/lexer.cpp`

**Function**: `isAllowedTokens()`

Added `:` to the list of allowed characters in STRING and KEYWORD tokens:

```cpp
if (!isalnum(c) && c != '_' && c != '.' && c != '/' && c != '-' && c != '=' && c != ':' && it->quoted == 0)
```

This allows the lexer to accept tokens like `127.0.0.1:3000` without throwing validation errors.

### 3. `src/models/headers/HttpRouter.hpp` & `src/models/srcs/HttpRouter.cpp`

Fixed C++98 compatibility issue with nested template:

```cpp
// Changed from:
std::map<std::string, std::map<std::string, HandlerFn>>

// To:
std::map<std::string, std::map<std::string, HandlerFn> >  // Space before >>
```

## Testing

Created `config/test_listen.conf` with all three listen formats.

**Test Results**:

```bash
$ ./pginx config/test_listen.conf
Parsing listen address:port: 127.0.0.1:3000
Parsing listen port: 8080
Parsing listen address:port: 0.0.0.0:9090
Server 0 listening on port 3000 (fd=3)
Server 1 listening on port 8080 (fd=4)
Server 2 listening on port 9090 (fd=5)
```

✅ All formats work correctly!

## Backward Compatibility

✅ **Fully backward compatible** - existing configs with just port numbers continue to work as before.

## Technical Details

The `ListenCtx` struct already had both fields:

```cpp
struct ListenCtx {
    u_int16_t port;
    std::string addr;
};
```

The parser now properly populates both fields from the configuration file.
