#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>

struct HttpResponse;

class HttpRequest {
protected:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> query;

    static std::string toLower(const std::string& s);

public:
    HttpRequest();
    virtual ~HttpRequest();

    // Accessors
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getQuery() const;

    // Setters (for parser)
    void setMethod(const std::string& m);
    void setPath(const std::string& p);
    void setVersion(const std::string& v);
    void addHeader(const std::string& k, const std::string& v);
    void appendBody(const std::string& data);
    void setQuery(const std::map<std::string, std::string>& q);

    // Helpers
    bool isChunked() const;
    size_t contentLength() const;
    static void parseQuery(const std::string& target, std::string& cleanPath, std::map<std::string, std::string>& outQuery);
    static bool parseHeaderLine(const std::string& line, std::string& k, std::string& v);

    // Validation and handling
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res) = 0;
};

// Request subclasses
class GetRequest : public HttpRequest {
public:
    GetRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class HeadRequest : public HttpRequest {
public:
    HeadRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class PostRequest : public HttpRequest {
public:
    PostRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class PutRequest : public HttpRequest {
public:
    PutRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class PatchRequest : public HttpRequest {
public:
    PatchRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class DeleteRequest : public HttpRequest {
public:
    DeleteRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

class OptionsRequest : public HttpRequest {
public:
    OptionsRequest();
    virtual bool validate(std::string& err) const;
    virtual void handle(HttpResponse& res);
};

// Factory function
HttpRequest* makeRequestByMethod(const std::string& method);

#endif
