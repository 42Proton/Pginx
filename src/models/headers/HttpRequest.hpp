#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Server.hpp"
#include "requestContext.hpp"
class HttpResponse;
class Server;

class HttpRequest
{
protected:
    const RequestContext _ctx;
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> query;
    bool enabledCgi;

    void handleGetOrHead(HttpResponse &res, bool includeBody, sockaddr_in &clientAddr, int epollFd);

private:
    // Prevent copying
    HttpRequest(const HttpRequest &other);
    HttpRequest &operator=(const HttpRequest &other);

public:
    HttpRequest(const RequestContext &ctx);
    virtual ~HttpRequest();

    // Accessors
    const std::string &getMethod() const;
    const std::string &getPath() const;
    const std::string &getVersion() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getBody() const;
    const std::map<std::string, std::string> &getQuery() const;

    // Setters (for parser)
    void setMethod(const std::string &m);
    void setPath(const std::string &p);
    void setVersion(const std::string &v);
    void addHeader(const std::string &k, const std::string &v);
    void appendBody(const std::string &data);
    void setQuery(const std::map<std::string, std::string> &q);
    void setEnabledCgi(bool enabled);

    // Helpers
    bool isChunked() const;
    size_t contentLength() const;
    static void parseQuery(const std::string &target, std::string &cleanPath,
                           std::map<std::string, std::string> &outQuery);
    static bool parseHeaderLine(const std::string &line, std::string &k, std::string &v);

    // Validation and handling
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd) = 0;
    };

// Request subclasses
class GetHeadRequest : public HttpRequest
{
public:
    GetHeadRequest(const RequestContext &ctx);
    virtual ~GetHeadRequest();

    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd);
};

class PostRequest : public HttpRequest
{
private:
    bool isPathSafe(const std::string &path) const;

public:
    PostRequest(const RequestContext &ctx);
    virtual ~PostRequest();

    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd);
};

class PutRequest : public HttpRequest
{
public:
    PutRequest();
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd);
};

class PatchRequest : public HttpRequest
{
public:
    PatchRequest();
    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd);
};

class DeleteRequest : public HttpRequest
{
private:
    bool isPathSafe(const std::string &fullPath) const;

public:
    DeleteRequest(const RequestContext &ctx);
    virtual ~DeleteRequest();

    virtual bool validate(std::string &err) const;
    virtual void handle(HttpResponse &res, sockaddr_in &clientAddr, int epollFd);
};

// Factory function
HttpRequest *makeRequestByMethod(const std::string &m, const RequestContext &ctx);

#endif
