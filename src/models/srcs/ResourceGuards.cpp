#include "ResourceGuards.hpp"

// RequestGuard implementation
RequestGuard::RequestGuard(HttpRequest *req) : request(req) {}

RequestGuard::~RequestGuard()
{
    delete request;
}

HttpRequest *RequestGuard::get() const
{
    return request;
}

HttpRequest *RequestGuard::operator->() const
{
    return request;
}

HttpRequest *RequestGuard::release()
{
    HttpRequest *temp = request;
    request = NULL;
    return temp;
}

bool RequestGuard::isValid() const
{
    return request != NULL;
}

// SocketGuard implementation
SocketGuard::SocketGuard(int socket_fd) : fd(socket_fd) {}

SocketGuard::~SocketGuard()
{
    if (fd >= 0)
    {
        close(fd);
    }
}

int SocketGuard::get() const
{
    return fd;
}

int SocketGuard::release()
{
    int temp = fd;
    fd = -1;
    return temp;
}

bool SocketGuard::isValid() const
{
    return fd >= 0;
}

// EpollGuard implementation
EpollGuard::EpollGuard(int epoll_fd) : fd(epoll_fd) {}

EpollGuard::~EpollGuard()
{
    if (fd >= 0)
    {
        close(fd);
    }
}

int EpollGuard::get() const
{
    return fd;
}

int EpollGuard::release()
{
    int temp = fd;
    fd = -1;
    return temp;
}

bool EpollGuard::isValid() const
{
    return fd >= 0;
}
