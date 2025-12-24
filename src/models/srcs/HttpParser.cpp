#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include "requestContext.hpp"
#include "Server.hpp"
#include "ResourceGuards.hpp"
#include <sstream>

HttpParser::HttpParser() : lastError("") {}

HttpParser::~HttpParser() {}

HttpRequest *HttpParser::parseRequest(const std::string &rawRequest, Server &server)
{
    clearError();
    size_t lineEnd = rawRequest.find("\r\n");
    if (lineEnd == std::string::npos)
    {
        lastError = "Invalid request format - no CRLF found";
        return NULL;
    }

    std::string requestLine = rawRequest.substr(0, lineEnd);
    std::string method, path, version;
    if (!parseRequestLine(requestLine, method, path, version))
    {
        return NULL;
    }

    std::string cleanPath;
    std::map<std::string, std::string> query;
    HttpRequest::parseQuery(path, cleanPath, query);

    const LocationConfig *location = server.findLocation(cleanPath);
    RequestContext ctx(server, location);

    RequestGuard request(makeRequestByMethod(method, ctx));
    if (!request.isValid())
    {
        lastError = "Unsupported HTTP method: " + method;
        return NULL;
    }

    request->setMethod(method);
    request->setPath(cleanPath);
    request->setVersion(version);
    request->setQuery(query);
    request->setEnabledCgi(location ? location->isCgiEnabled() : false);
 
    size_t headerStart = lineEnd + 2;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        headerEnd = rawRequest.length();
    }

    if (headerEnd > headerStart)
    {
        std::string headerSection = rawRequest.substr(headerStart, headerEnd - headerStart);
        if (!parseHeaders(headerSection, request.get()))
        {
            return NULL;
        }
    }

    if (headerEnd + 4 < rawRequest.length())
    {
        std::string body = rawRequest.substr(headerEnd + 4);
        parseBody(body, request.get());
    }

    return request.release();
}

bool HttpParser::parseRequestLine(const std::string &line, std::string &method, std::string &path, std::string &version)
{
    std::istringstream iss(line);
    if (!(iss >> method >> path >> version))
    {
        lastError = "Invalid request line format";
        return false;
    }

    if (!isValidMethod(method) || !isValidPath(path) || !isValidVersion(version))
    {
        return false;
    }

    return true;
}

bool HttpParser::parseHeaders(const std::string &headerSection, HttpRequest *request)
{
    std::istringstream headerStream(headerSection);
    std::string line;

    while (std::getline(headerStream, line))
    {
        if (!line.empty() && line[line.length() - 1] == '\r')
        {
            line.erase(line.length() - 1);
        }
        if (line.empty())
            break;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            while (!value.empty() && value[0] == ' ')
            {
                value.erase(0, 1);
            }

            request->addHeader(key, value);
        }
    }

    return true;
}

bool HttpParser::parseChunkedBody(const std::string &body, HttpRequest *request)
{
    
    std::string unchunkedBody;
    size_t pos = 0;
    
    while (pos < body.length())
    {
        size_t lineEnd = body.find("\r\n", pos);
        if (lineEnd == std::string::npos)
        {
            lastError = "Invalid chunked encoding: no CRLF after chunk size";
            return false;
        }

        std::string sizeLine = body.substr(pos, lineEnd - pos);
        size_t semicolon = sizeLine.find(';');
        if (semicolon != std::string::npos)
        {
            sizeLine = sizeLine.substr(0, semicolon);
        }

        std::istringstream hexStream(sizeLine);
        unsigned long chunkSize;
        hexStream >> std::hex >> chunkSize;
        if (hexStream.fail() || !hexStream.eof())
        {
            lastError = "Invalid chunk size: not a valid hex number";
            return false;
        }
        pos = lineEnd + 2;
        
        if (chunkSize == 0)
        {
            while (pos < body.length())
            {
                size_t trailerEnd = body.find("\r\n", pos);
                if (trailerEnd == std::string::npos)
                    break;
                if (trailerEnd == pos)
                    break;
                pos = trailerEnd + 2;
            }
            request->appendBody(unchunkedBody);
            
            return true;
        }
        if (pos + chunkSize > body.length())
        {
            lastError = "Invalid chunk: data shorter than specified size";
            return false;
        }
        std::string chunkData = body.substr(pos, chunkSize);
        unchunkedBody.append(chunkData);
        pos += chunkSize;
        if (pos + 2 <= body.length() && 
            body[pos] == '\r' && body[pos + 1] == '\n')
        {
            pos += 2;
        }
        else
        {
            lastError = "Invalid chunk: no CRLF after chunk data";
            return false;
        }
    }
    lastError = "Invalid chunked encoding: no terminating chunk found";
    return false;
}

bool HttpParser::parseBody(const std::string &body, HttpRequest *request)
{
    if (request->isChunked())
    {
        if (parseChunkedBody(body, request))
            return true;
        else
        {
            lastError = "Failed to parse chunked body";
            return false;
        }
    }
    request->appendBody(body);
    return true;
}

bool HttpParser::isValidMethod(const std::string &method)
{
    return method == "GET" || method == "POST" ||
           method == "DELETE" || method == "HEAD";
}

bool HttpParser::isValidPath(const std::string &path)
{
    return !path.empty() && path[0] == '/';
}

bool HttpParser::isValidVersion(const std::string &version)
{
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}

void HttpParser::clearError()
{
    lastError.clear();
}
