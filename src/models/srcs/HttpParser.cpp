#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include "requestContext.hpp"
#include "Server.hpp"
#include <sstream>

HttpParser::HttpParser() : lastError("") {}

HttpParser::~HttpParser() {}

HttpRequest* HttpParser::parseRequest(const std::string& rawRequest, Server& server) {
    clearError();
    
    // Find request line (first line)
    size_t lineEnd = rawRequest.find("\r\n");
    if (lineEnd == std::string::npos) {
        lastError = "Invalid request format - no CRLF found";
        return NULL;
    }
    
    std::string requestLine = rawRequest.substr(0, lineEnd);
    
    // Parse method, path, version
    std::string method, path, version;
    if (!parseRequestLine(requestLine, method, path, version)) {
        return NULL;
    }
    
    // Parse query string to get clean path for location matching
    std::string cleanPath;
    std::map<std::string, std::string> query;
    HttpRequest::parseQuery(path, cleanPath, query);
    
    // Find matching location for this path
    const LocationConfig* location = server.findLocation(cleanPath);
    
    // Create RequestContext with server and location
    RequestContext ctx(server, location);
    
    // Create appropriate request object
    HttpRequest* request = makeRequestByMethod(method, ctx);
    if (!request) {
        lastError = "Unsupported HTTP method: " + method;
        return NULL;
    }
    
    request->setMethod(method);
    request->setPath(cleanPath);
    request->setVersion(version);
    request->setQuery(query);
    
    // Parse headers
    size_t headerStart = lineEnd + 2;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        headerEnd = rawRequest.length();
    }
    
    if (headerEnd > headerStart) {
        std::string headerSection = rawRequest.substr(headerStart, headerEnd - headerStart);
        if (!parseHeaders(headerSection, request)) {
            delete request;
            return NULL;
        }
    }
    
    // Parse body if present
    if (headerEnd + 4 < rawRequest.length()) {
        std::string body = rawRequest.substr(headerEnd + 4);
        parseBody(body, request);
    }
    
    return request;
}

bool HttpParser::parseRequestLine(const std::string& line, std::string& method, std::string& path, std::string& version) {
    std::istringstream iss(line);
    if (!(iss >> method >> path >> version)) {
        lastError = "Invalid request line format";
        return false;
    }
    
    if (!isValidMethod(method) || !isValidPath(path) || !isValidVersion(version)) {
        return false;
    }
    
    return true;
}

bool HttpParser::parseHeaders(const std::string& headerSection, HttpRequest* request) {
    std::istringstream headerStream(headerSection);
    std::string line;
    
    while (std::getline(headerStream, line)) {
        // Remove carriage return if present
        if (!line.empty() && line[line.length()-1] == '\r') {
            line.erase(line.length()-1);
        }
        if (line.empty()) break;
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim leading whitespace from value
            while (!value.empty() && value[0] == ' ') {
                value.erase(0, 1);
            }
            
            request->addHeader(key, value);
        }
    }
    
    return true;
}

bool HttpParser::parseBody(const std::string& body, HttpRequest* request) {
    request->appendBody(body);
    return true;
}

bool HttpParser::isValidMethod(const std::string& method) {
    return method == "GET" || method == "POST" || method == "PUT" || 
           method == "DELETE" || method == "HEAD" || method == "OPTIONS";
}

bool HttpParser::isValidPath(const std::string& path) {
    return !path.empty() && path[0] == '/';
}

bool HttpParser::isValidVersion(const std::string& version) {
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}

bool HttpParser::hasError() const {
    return !lastError.empty();
}

std::string HttpParser::getLastError() const {
    return lastError;
}

void HttpParser::clearError() {
    lastError.clear();
}
