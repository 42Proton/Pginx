#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include <sstream>
#include <string>
#include "requestContext.hpp"

HttpResponse::HttpResponse() : statusCode(200), statusMessage("OK") {}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatus(int code, const std::string& reason) {
    statusCode = code;
    statusMessage = reason;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HttpResponse::setBody(const std::string& b) {
    body = b;
}

void HttpResponse::setVersion(const std::string &v) {
    version = v;
}

std::string HttpResponse::build() const {
    std::ostringstream response;

    // Start line: HTTP version + status code + message
    response << version << " " << statusCode << " " << statusMessage << "\r\n";

    // Headers
    std::map<std::string, std::string>::const_iterator it = headers.begin();
    for (; it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    // Blank line separating headers and body
    response << "\r\n";

    // Body
    response << body;

    return response.str();
}

static std::string getStatusMessage(int code) {
    switch (code) {
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 408:
            return "Request Timeout";
        case 413:
            return "Payload Too Large";
        case 431:
            return "Request Header Fields Too Large";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        default: return "Error";
    }
}

void HttpResponse::setError(int code, const std::string& reason) {
    setStatus(code, reason);
    std::ostringstream content;
    content << "<html><body><h1>Error " << code << " - " << reason << "</h1></body></html>";
    setBody(content.str());
    
    std::ostringstream lenStream;
    lenStream << body.size();
    setHeader("Content-Length", lenStream.str());
    setHeader("Content-Type", "text/html");
}

void HttpResponse::setErrorFromContext(int code, const RequestContext &ctx) {
    std::string content;
    
    try {
        content = ctx.getErrorPageContent(code);
    }
    catch (const std::exception &e) {
        std::cerr << "Error loading page: " << e.what() << '\n';
        std::ostringstream fallback;
        fallback << "<html><body><h1>Error " << code << "</h1></body></html>";
        content = fallback.str();
    }

    setStatus(code, getStatusMessage(code));
    std::ostringstream lenStream;
    lenStream << content.size();
    setHeader("Content-Length", lenStream.str());
    setHeader("Content-Type", "text/html");
    setBody(content);
}


std::string HttpResponse::getHostHeader() const {
    std::map<std::string, std::string>::const_iterator it = headers.find("Host");
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}
