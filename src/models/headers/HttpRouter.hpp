#ifndef HTTPROUTER_HPP
#define HTTPROUTER_HPP

#include <map>
#include <string>

class HttpRequest;
struct HttpResponse;

typedef void (*HandlerFn)(const HttpRequest &, HttpResponse &);

class Router
{
    std::map<std::string, std::map<std::string, HandlerFn>> table;

  public:
    void add(const std::string &method, const std::string &path, HandlerFn h);
    bool dispatch(const HttpRequest &req, HttpResponse &res) const;
    std::string allowForPath(const std::string &path) const;
};

// Demo handler functions
void handleRoot(const HttpRequest &req, HttpResponse &res);
void handleEchoGet(const HttpRequest &req, HttpResponse &res);
void handleEchoPost(const HttpRequest &req, HttpResponse &res);
void handleOptions(const HttpRequest &req, HttpResponse &res);

#endif
