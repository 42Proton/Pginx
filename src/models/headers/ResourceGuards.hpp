#ifndef RESOURCEGUARDS_HPP
#define RESOURCEGUARDS_HPP

#include "HttpRequest.hpp"
#include <unistd.h>

// RAII guard for HttpRequest pointers - auto-deletes on scope exit
class RequestGuard
{
private:
    HttpRequest *request;

    // Prevent copying (C++98 style)
    RequestGuard(const RequestGuard &);
    RequestGuard &operator=(const RequestGuard &);

public:
    explicit RequestGuard(HttpRequest *req = NULL);
    ~RequestGuard();
    HttpRequest *get() const;
    HttpRequest *operator->() const;
    HttpRequest *release();
    bool isValid() const;
};

// RAII guard for socket FDs - auto-closes on scope exit
class SocketGuard
{
private:
    int fd;

    // Prevent copying (C++98 style)
    SocketGuard(const SocketGuard &);
    SocketGuard &operator=(const SocketGuard &);

public:
    explicit SocketGuard(int socket_fd = -1);
    ~SocketGuard();
    int get() const;
    int release();
    bool isValid() const;
};

// RAII guard for epoll FDs - auto-closes on scope exit
class EpollGuard
{
private:
    int fd;

    // Prevent copying (C++98 style)
    EpollGuard(const EpollGuard &);
    EpollGuard &operator=(const EpollGuard &);

public:
    explicit EpollGuard(int epoll_fd = -1);
    ~EpollGuard();
    int get() const;
    int release();
    bool isValid() const;
};

#endif // RESOURCEGUARDS_HPP
