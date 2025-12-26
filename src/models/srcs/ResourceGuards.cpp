#include "ResourceGuards.hpp"

// RequestGuard implementation
RequestGuard::RequestGuard(HttpRequest* req) : request(req) {}

// Copy assignment operator (private - not meant to be used)
// This prevents accidental copying which would lead to double-deletion
RequestGuard& RequestGuard::operator=(const RequestGuard& other) {
  if (this != &other) {
    // Delete current resource
    delete request;
    // Copy the pointer (shallow copy - both would point to same object)
    // This is dangerous and why this operator is private!
    request = other.request;
  }
  return *this;
}

RequestGuard::~RequestGuard() {
  delete request;
}

HttpRequest* RequestGuard::get() const {
  return request;
}

HttpRequest* RequestGuard::operator->() const {
  return request;
}

HttpRequest* RequestGuard::release() {
  HttpRequest* temp = request;
  request = NULL;
  return temp;
}

bool RequestGuard::isValid() const {
  return request != NULL;
}

// SocketGuard implementation
SocketGuard::SocketGuard(int socket_fd) : fd(socket_fd) {}

// Copy assignment operator (private - not meant to be used)
SocketGuard& SocketGuard::operator=(const SocketGuard& other) {
  if (this != &other) {
    // Close current file descriptor
    if (fd >= 0) {
      close(fd);
    }
    // Copy the fd (both would manage the same fd - dangerous!)
    fd = other.fd;
  }
  return *this;
}

SocketGuard::~SocketGuard() {
  if (fd >= 0) {
    close(fd);
  }
}

int SocketGuard::get() const {
  return fd;
}

int SocketGuard::release() {
  int temp = fd;
  fd = -1;
  return temp;
}

bool SocketGuard::isValid() const {
  return fd >= 0;
}

// EpollGuard implementation
EpollGuard::EpollGuard(int epoll_fd) : fd(epoll_fd) {}

// Copy assignment operator (private - not meant to be used)
EpollGuard& EpollGuard::operator=(const EpollGuard& other) {
  if (this != &other) {
    // Close current file descriptor
    if (fd >= 0) {
      close(fd);
    }
    // Copy the fd (both would manage the same fd - dangerous!)
    fd = other.fd;
  }
  return *this;
}

EpollGuard::~EpollGuard() {
  if (fd >= 0) {
    close(fd);
  }
}

int EpollGuard::get() const {
  return fd;
}

int EpollGuard::release() {
  int temp = fd;
  fd = -1;
  return temp;
}

bool EpollGuard::isValid() const {
  return fd >= 0;
}
