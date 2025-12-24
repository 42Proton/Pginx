# CGI File Descriptor Management and Epoll-Based I/O

## Problem
The original CGI implementation had several issues:

1. **File descriptor leaks**: The child process inherited the epoll FD from the parent, which it didn't need and never closed
2. **Blocking I/O**: Using synchronous `read()` and `write()` calls blocked the server thread
3. **No proper timeout**: Blocking I/O made it difficult to implement proper timeouts
4. **Using `exit()` instead of `_exit()`**: After `fork()`, child processes should use `_exit()` to avoid flushing parent's buffers

## Solutions Applied

### 1. Epoll-Based Non-Blocking I/O

The CGI I/O now uses epoll for non-blocking operations:

```cpp
std::string readCgiResponse(const std::string &inputData, int stdinPipe, 
                            int stdoutPipe, int epollFd, pid_t childPid)
```

**Key features:**
- Sets pipes to non-blocking mode using `fcntl()`
- Adds both stdin and stdout pipes to epoll with edge-triggered mode (`EPOLLET`)
- Uses `epoll_wait()` with timeout for monitoring pipe events
- Handles `EAGAIN`/`EWOULDBLOCK` errors properly for non-blocking I/O
- Implements proper timeout mechanism (5 seconds) with `SIGKILL` to child

**Benefits:**
- Server doesn't block while waiting for CGI output
- Can handle multiple CGI requests concurrently
- Proper timeout enforcement
- Better resource management

### 2. Close inherited epoll FD in child process

```cpp
// Close epoll fd inherited from parent - child doesn't need it
if (epollFd != -1) {
    close(epollFd);
}
```

The child process doesn't need the epoll file descriptor, so we close it immediately after fork.

### 3. Use `_exit()` instead of `exit()` in child

After `fork()`, the child process should use `_exit()` instead of `exit()` to:
- Avoid flushing parent's I/O buffers
- Avoid calling atexit() handlers from parent
- Properly terminate without side effects

```cpp
if (chdir(scriptDir.c_str()) == -1) {
    _exit(1);  // Changed from exit(1)
}

// ... after execve fails
freeCharArray(envp, envVars.size());
_exit(1);  // Changed from exit(1)
```

### 4. Proper Timeout Handling

The timeout is now enforced during the I/O operations:
- Checks elapsed time during `epoll_wait()` loop
- If timeout exceeded (5 seconds):
  - Removes pipes from epoll
  - Closes all pipe file descriptors
  - Kills child process with `SIGKILL`
  - Waits for child to clean up
  - Throws `CgiTimeoutException`

## File Descriptor Lifecycle in CGI Execution

### Parent Process:
1. Creates pipes: `stdinPipe[2]`, `stdoutPipe[2]`
2. Forks child
3. Closes unused ends: `stdinPipe[0]`, `stdoutPipe[1]`
4. Sets remaining pipes to non-blocking mode
5. Adds pipes to epoll:
   - `stdinPipe[1]` with `EPOLLOUT` (for writing input data)
   - `stdoutPipe[0]` with `EPOLLIN` (for reading CGI output)
6. Enters epoll event loop:
   - Writes input data to stdin when `EPOLLOUT` is ready
   - Reads CGI output from stdout when `EPOLLIN` is ready
   - Handles `EPOLLERR` and `EPOLLHUP` events
   - Monitors timeout
7. When all output is read (EOF on stdout):
   - Removes pipes from epoll
   - Closes all pipes
8. Waits for child process to finish with `waitpid()`

### Child Process:
1. Redirects stdin/stdout using `dup2()`
2. Closes all pipe FDs (no longer needed after dup2)
3. **Closes inherited epoll FD**
4. Changes directory to script location
5. Executes CGI script with `execve()`
6. On any error, uses `_exit(1)` to terminate cleanly

## Epoll Event Handling

### EPOLLOUT on stdin pipe:
- Writes input data to CGI in non-blocking manner
- Handles partial writes
- Closes stdin after all data is written

### EPOLLIN on stdout pipe:
- Reads CGI output in chunks (4KB buffer)
- Appends to output string
- Continues reading until `EAGAIN` or EOF
- Closes stdout on EOF

### EPOLLERR / EPOLLHUP:
- Removes affected FD from epoll
- Closes the pipe
- Continues if stdout has been read completely

## Benefits
- **Non-blocking**: Server can handle other requests while CGI executes
- **Proper timeout**: CGI scripts that hang are killed after 5 seconds
- **No FD leaks**: All file descriptors properly closed in all code paths
- **Scalable**: Can handle multiple concurrent CGI requests
- **POSIX compliant**: Follows best practices for fork/exec and non-blocking I/O
- **Error handling**: Properly handles all error conditions (EAGAIN, EWOULDBLOCK, EPOLLHUP, etc.)

## Edge-Triggered Mode

Using `EPOLLET` (edge-triggered) mode ensures:
- Events are only reported once when state changes
- Must read/write all available data when event occurs
- More efficient than level-triggered mode
- Requires proper handling of `EAGAIN`/`EWOULDBLOCK`
