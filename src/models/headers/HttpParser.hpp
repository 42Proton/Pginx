#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>
#include <map>

class HttpRequest;
class Server;

class HttpParser
{
private:
    std::string lastError;

    // Parsing utilities
    bool parseRequestLine(const std::string &line, std::string &method, std::string &path, std::string &version);
    bool parseHeaders(const std::string &headerSection, HttpRequest *request);
    bool parseBody(const std::string &body, HttpRequest *request);
    bool parseChunkedBody(const std::string &body, HttpRequest *request);

    // Validation helpers
    bool isValidMethod(const std::string &method);
    bool isValidPath(const std::string &path);
    bool isValidVersion(const std::string &version);

public:
    HttpParser();
    ~HttpParser();

    // Main parsing method
    HttpRequest *parseRequest(const std::string &rawRequest, Server &server);

    // Error handling
    bool hasError() const;
    std::string getLastError() const;
    void clearError();
};

#endif
