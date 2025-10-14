#include "HttpRouter.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpUtils.hpp"
#include <algorithm>
#include <vector>

void Router::add(const std::string &method, const std::string &path, HandlerFn h)
{
    table[toLowerStr(method)][path] = h;
}

bool Router::dispatch(const HttpRequest &req, HttpResponse &res) const
{
    std::string ml = toLowerStr(req.getMethod());
    std::map<std::string, std::map<std::string, HandlerFn> >::const_iterator mit = table.find(ml);
    if (mit == table.end())
        return false;
    std::map<std::string, HandlerFn>::const_iterator pit = mit->second.find(req.getPath());
    if (pit == mit->second.end())
        return false;
    pit->second(req, res);
    return true;
}

std::string Router::allowForPath(const std::string &path) const
{
    std::vector<std::string> methods;
    for (std::map<std::string, std::map<std::string, HandlerFn> >::const_iterator mit = table.begin();
         mit != table.end(); ++mit)
    {
        if (mit->second.find(path) != mit->second.end())
        {
            std::string m = mit->first;
            for (size_t i = 0; i < m.size(); ++i)
                m[i] = std::toupper(static_cast<unsigned char>(m[i]));
            methods.push_back(m);
        }
    }
    std::string result;
    for (size_t i = 0; i < methods.size(); ++i)
    {
        if (i > 0)
            result += ",";
        result += methods[i];
    }
    return result;
}

// Demo handler implementations
void handleRoot(const HttpRequest &req, HttpResponse &res)
{
    (void)req;
    res.status = 200;
    res.reason = "OK";
    res.body = "<html><body><h1>Pginx HTTP/1.1 Server</h1><p>Welcome!</p></body></html>";
    res.setHeader("Content-Type", "text/html");
    res.setHeader("Content-Length", itoa_custom(res.body.size()));
}

void handleEchoGet(const HttpRequest &req, HttpResponse &res)
{
    res.status = 200;
    res.reason = "OK";
    std::string text = "Query params:\n";
    const std::map<std::string, std::string> &q = req.getQuery();
    for (std::map<std::string, std::string>::const_iterator it = q.begin(); it != q.end(); ++it)
    {
        text += it->first + " = " + it->second + "\n";
    }
    res.body = text;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Length", itoa_custom(res.body.size()));
}

void handleEchoPost(const HttpRequest &req, HttpResponse &res)
{
    res.status = 200;
    res.reason = "OK";
    res.body = req.getBody();
    res.setHeader("Content-Type", "application/octet-stream");
    res.setHeader("Content-Length", itoa_custom(res.body.size()));
}

void handleOptions(const HttpRequest &req, HttpResponse &res)
{
    (void)req;
    res.status = 200;
    res.reason = "OK";
    res.body = "";
    res.setHeader("Allow", "GET,POST,PUT,PATCH,DELETE,OPTIONS,HEAD");
    res.setHeader("Content-Length", "0");
    std::map<std::string, std::string>::const_iterator it = req.getHeaders().find("origin");
    if (it != req.getHeaders().end())
    {
        res.setHeader("Access-Control-Allow-Origin", "*");
        res.setHeader("Access-Control-Allow-Methods", "GET,POST,PUT,PATCH,DELETE,OPTIONS,HEAD");
        res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.setHeader("Access-Control-Max-Age", "3600");
    }
}
