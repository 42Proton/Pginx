// Example: How to integrate HTTP classes into your SocketManager
// This shows the concept - you'll need to adapt to your actual parsing logic

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpRouter.hpp"
#include "HttpUtils.hpp"

// Example of a simple request parser (you can improve this)
HttpRequest *parseSimpleRequest(const std::string &rawRequest)
{
    // Find first line
    size_t lineEnd = rawRequest.find("\r\n");
    if (lineEnd == std::string::npos)
        return NULL;

    std::string requestLine = rawRequest.substr(0, lineEnd);

    // Parse: METHOD /path HTTP/1.1
    size_t firstSpace = requestLine.find(' ');
    size_t secondSpace = requestLine.find(' ', firstSpace + 1);
    if (firstSpace == std::string::npos || secondSpace == std::string::npos)
        return NULL;

    std::string method = requestLine.substr(0, firstSpace);
    std::string target = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    std::string version = requestLine.substr(secondSpace + 1);

    // Create request object
    HttpRequest *req = makeRequestByMethod(method);
    if (!req)
        return NULL;

    // Parse path and query
    std::string path;
    std::map<std::string, std::string> query;
    HttpRequest::parseQuery(target, path, query);

    req->setPath(path);
    req->setVersion(version);
    req->setQuery(query);

    // Parse headers
    size_t headerStart = lineEnd + 2;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd != std::string::npos)
    {
        std::string headerBlock = rawRequest.substr(headerStart, headerEnd - headerStart);

        // Parse each header line
        size_t pos = 0;
        while (pos < headerBlock.size())
        {
            size_t nextLine = headerBlock.find("\r\n", pos);
            if (nextLine == std::string::npos)
                nextLine = headerBlock.size();

            std::string headerLine = headerBlock.substr(pos, nextLine - pos);
            std::string key, value;
            if (HttpRequest::parseHeaderLine(headerLine, key, value))
            {
                req->addHeader(key, value);
            }

            pos = nextLine + 2;
        }

        // Get body if present
        size_t bodyStart = headerEnd + 4;
        if (bodyStart < rawRequest.size())
        {
            req->appendBody(rawRequest.substr(bodyStart));
        }
    }

    return req;
}

// Example: In your SocketManager::handleRequest() method
void exampleUsageInSocketManager(const std::string &rawRequest, std::string &responseOut)
{
    // 1. Parse the raw HTTP request
    HttpRequest *req = parseSimpleRequest(rawRequest);
    if (!req)
    {
        responseOut = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        return;
    }

    // 2. Validate the request
    std::string validationError;
    if (!req->validate(validationError))
    {
        HttpResponse res;
        res.status = 400;
        res.reason = "Bad Request";
        res.body = validationError;
        res.setHeader("Content-Type", "text/plain");
        res.setHeader("Content-Length", itoa_custom(res.body.size()));
        responseOut = res.serialize(false);
        delete req;
        return;
    }

    // 3. Create router and register your routes
    Router router;
    router.add("GET", "/", handleRoot);
    router.add("GET", "/echo", handleEchoGet);
    router.add("POST", "/echo", handleEchoPost);
    router.add("OPTIONS", "*", handleOptions);

    // 4. Dispatch request to handler
    HttpResponse res;
    bool found = router.dispatch(*req, res);

    if (!found)
    {
        // Check if path exists but wrong method (405)
        std::string allowedMethods = router.allowForPath(req->getPath());
        if (!allowedMethods.empty())
        {
            res.status = 405;
            res.reason = "Method Not Allowed";
            res.setHeader("Allow", allowedMethods);
            res.body = "Method Not Allowed";
        }
        else
        {
            // Path not found (404)
            res.status = 404;
            res.reason = "Not Found";
            res.body = "Not Found";
        }
        res.setHeader("Content-Type", "text/plain");
        res.setHeader("Content-Length", itoa_custom(res.body.size()));
    }

    // 5. Serialize response
    bool isHeadRequest = (req->getMethod() == "HEAD");
    responseOut = res.serialize(isHeadRequest);

    // 6. Cleanup
    delete req;
}

// Example of a custom handler function
void myCustomHandler(const HttpRequest &req, HttpResponse &res)
{
    res.status = 200;
    res.reason = "OK";

    // Access request data
    std::string body = "You requested: " + req.getPath() + "\n";
    body += "Method: " + req.getMethod() + "\n";

    // Access query parameters
    const std::map<std::string, std::string> &params = req.getQuery();
    if (!params.empty())
    {
        body += "Query parameters:\n";
        for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it)
        {
            body += "  " + it->first + " = " + it->second + "\n";
        }
    }

    // Access headers
    const std::map<std::string, std::string> &headers = req.getHeaders();
    std::map<std::string, std::string>::const_iterator userAgent = headers.find("user-agent");
    if (userAgent != headers.end())
    {
        body += "User-Agent: " + userAgent->second + "\n";
    }

    res.body = body;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Length", itoa_custom(res.body.size()));
}
