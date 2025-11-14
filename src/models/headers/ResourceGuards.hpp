#ifndef RESOURCEGUARDS_HPP
#define RESOURCEGUARDS_HPP

#include "HttpRequest.hpp"
#include <unistd.h>

/**
 * @brief RAII guard for HttpRequest pointers
 *
 * Automatically manages the lifetime of dynamically allocated HttpRequest objects.
 * Prevents memory leaks by ensuring delete is called when guard goes out of scope.
 *
 * Usage:
 *   RequestGuard guard(new GetHeadRequest());
 *   // ... use guard.get() or guard->method()
 *   // Automatic cleanup when guard goes out of scope
 *   return guard.release(); // Transfer ownership to caller
 */
class RequestGuard
{
private:
    HttpRequest *request;

    // Prevent copying (C++98 style)
    RequestGuard(const RequestGuard &);
    RequestGuard &operator=(const RequestGuard &);

public:
    /**
     * @brief Construct guard taking ownership of request pointer
     * @param req Pointer to HttpRequest (can be NULL)
     */
    explicit RequestGuard(HttpRequest *req = NULL) : request(req) {}

    /**
     * @brief Destructor - automatically deletes managed request
     */
    ~RequestGuard()
    {
        delete request;
    }

    /**
     * @brief Get raw pointer without transferring ownership
     * @return Pointer to managed HttpRequest (may be NULL)
     */
    HttpRequest *get() const
    {
        return request;
    }

    /**
     * @brief Arrow operator for convenient member access
     * @return Pointer to managed HttpRequest
     */
    HttpRequest *operator->() const
    {
        return request;
    }

    /**
     * @brief Release ownership and return pointer
     * @return Pointer to HttpRequest (caller now owns it)
     *
     * After release(), the guard no longer manages the pointer.
     * Caller is responsible for cleanup.
     */
    HttpRequest *release()
    {
        HttpRequest *temp = request;
        request = NULL;
        return temp;
    }

    /**
     * @brief Check if guard manages a valid pointer
     * @return true if request is not NULL
     */
    bool isValid() const
    {
        return request != NULL;
    }
};

/**
 * @brief RAII guard for socket file descriptors
 *
 * Automatically manages socket lifetime by closing the FD when guard
 * goes out of scope. Prevents socket leaks.
 *
 * Usage:
 *   SocketGuard guard(socket(AF_INET, SOCK_STREAM, 0));
 *   if (!guard.isValid()) { handle error }
 *   // ... use guard.get() for socket operations
 *   // Automatic close() when guard goes out of scope
 *   int fd = guard.release(); // Transfer ownership to caller
 */
class SocketGuard
{
private:
    int fd;

    // Prevent copying (C++98 style)
    SocketGuard(const SocketGuard &);
    SocketGuard &operator=(const SocketGuard &);

public:
    /**
     * @brief Construct guard taking ownership of socket FD
     * @param socket_fd Socket file descriptor (-1 for invalid)
     */
    explicit SocketGuard(int socket_fd = -1) : fd(socket_fd) {}

    /**
     * @brief Destructor - automatically closes socket
     */
    ~SocketGuard()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    /**
     * @brief Get socket FD without transferring ownership
     * @return Socket file descriptor (-1 if invalid)
     */
    int get() const
    {
        return fd;
    }

    /**
     * @brief Release ownership and return FD
     * @return Socket file descriptor (caller now owns it)
     *
     * After release(), the guard no longer manages the FD.
     * Caller is responsible for closing.
     */
    int release()
    {
        int temp = fd;
        fd = -1;
        return temp;
    }

    /**
     * @brief Check if guard manages a valid socket
     * @return true if fd >= 0
     */
    bool isValid() const
    {
        return fd >= 0;
    }
};

/**
 * @brief RAII guard for epoll file descriptors
 *
 * Automatically manages epoll lifetime by closing the FD when guard
 * goes out of scope. Prevents epoll leaks.
 *
 * Usage:
 *   EpollGuard guard(epoll_create1(0));
 *   if (!guard.isValid()) { handle error }
 *   // ... use guard.get() for epoll operations
 *   // Automatic close() when guard goes out of scope
 *   int fd = guard.release(); // Transfer ownership to caller
 */
class EpollGuard
{
private:
    int fd;

    // Prevent copying (C++98 style)
    EpollGuard(const EpollGuard &);
    EpollGuard &operator=(const EpollGuard &);

public:
    /**
     * @brief Construct guard taking ownership of epoll FD
     * @param epoll_fd Epoll file descriptor (-1 for invalid)
     */
    explicit EpollGuard(int epoll_fd = -1) : fd(epoll_fd) {}

    /**
     * @brief Destructor - automatically closes epoll FD
     */
    ~EpollGuard()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    /**
     * @brief Get epoll FD without transferring ownership
     * @return Epoll file descriptor (-1 if invalid)
     */
    int get() const
    {
        return fd;
    }

    /**
     * @brief Release ownership and return FD
     * @return Epoll file descriptor (caller now owns it)
     *
     * After release(), the guard no longer manages the FD.
     * Caller is responsible for closing.
     */
    int release()
    {
        int temp = fd;
        fd = -1;
        return temp;
    }

    /**
     * @brief Check if guard manages a valid epoll FD
     * @return true if fd >= 0
     */
    bool isValid() const
    {
        return fd >= 0;
    }
};

#endif // RESOURCEGUARDS_HPP
