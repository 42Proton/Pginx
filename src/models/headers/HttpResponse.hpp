#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

struct HttpResponse {
    int status;
    std::string reason;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse();
    void setHeader(const std::string& k, const std::string& v);
    std::string serialize(bool headOnly) const;
};

#endif
