#include "HttpResponse.hpp"
#include "HttpUtils.hpp"

HttpResponse::HttpResponse() : status(200), reason("OK") {}

void HttpResponse::setHeader(const std::string& k, const std::string& v) {
    headers[k] = v;
}

std::string HttpResponse::serialize(bool headOnly) const {
    std::string resp = "HTTP/1.1 " + itoa_int(status) + " " + reason + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        resp += it->first + ": " + it->second + "\r\n";
    }
    resp += "\r\n";
    if (!headOnly) {
        resp += body;
    }
    return resp;
}
