#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpUtils.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <sys/stat.h>

// HttpRequest base class implementation
HttpRequest::HttpRequest(const RequestContext &ctx) : _ctx(ctx)
{
}

HttpRequest::~HttpRequest()
{
}

// Accessors
const std::string &HttpRequest::getMethod() const
{
    return method;
}

const std::string &HttpRequest::getPath() const
{
    return path;
}

const std::string &HttpRequest::getVersion() const
{
    return version;
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
    return headers;
}

const std::string &HttpRequest::getBody() const
{
    return body;
}

const std::map<std::string, std::string> &HttpRequest::getQuery() const
{
    return query;
}

// Setters (used by your parser)
void HttpRequest::setMethod(const std::string &m)
{
    method = m;
}

void HttpRequest::setPath(const std::string &p)
{
    path = p;
}

void HttpRequest::setVersion(const std::string &v)
{
    version = v;
}

void HttpRequest::addHeader(const std::string &k, const std::string &v)
{
    headers[k] = v;
}

void HttpRequest::appendBody(const std::string &data)
{
    body.append(data);
}

void HttpRequest::setQuery(const std::map<std::string, std::string> &q)
{
    query = q;
}

bool HttpRequest::validate(std::string &err) const
{
    (void)err;
    return true;
}

// Helpers for request handling
bool HttpRequest::isChunked() const
{
    std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
    if (it == headers.end())
        return false;

    std::string value = toLowerStr(it->second);
    return (value.find("chunked") != std::string::npos);
}

size_t HttpRequest::contentLength() const
{
    std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
    if (it == headers.end())
        return 0;
    return safeAtoi(it->second);
}

void HttpRequest::parseQuery(const std::string &target, std::string &cleanPath,
                             std::map<std::string, std::string> &outQuery)
{
    size_t qpos = target.find('?');
    if (qpos == std::string::npos)
    {
        cleanPath = target;
        return;
    }
    cleanPath = target.substr(0, qpos);
    std::string qstr = target.substr(qpos + 1);

    size_t start = 0;
    while (start < qstr.size())
    {
        size_t eq = qstr.find('=', start);
        size_t amp = qstr.find('&', start);
        std::string key, val;

        if (eq == std::string::npos || (amp != std::string::npos && amp < eq))
        {
            key = urlDecode(qstr.substr(start, (amp == std::string::npos ? qstr.size() : amp) - start));
            val = "";
        }
        else
        {
            key = urlDecode(qstr.substr(start, eq - start));
            val = urlDecode(qstr.substr(eq + 1, (amp == std::string::npos ? qstr.size() : amp) - eq - 1));
        }

        if (!key.empty())
            outQuery[key] = val;
        if (amp == std::string::npos)
            break;
        start = amp + 1;
    }
}

bool HttpRequest::parseHeaderLine(const std::string &line, std::string &k, std::string &v)
{
    size_t colon = line.find(':');
    if (colon == std::string::npos)
        return false;
    k = toLowerStr(trim(line.substr(0, colon)));
    v = trim(line.substr(colon + 1));
    return true;
}

HttpRequest *makeRequestByMethod(const std::string &method, const RequestContext &ctx)
{
    if (method == "GET" || method == "HEAD")
        return new GetHeadRequest(ctx);
    if (method == "POST")
        return new PostRequest(ctx);
    // if (method == "put")
    //     return new PutRequest();
    // if (method == "patch")
    //     return new PatchRequest();
    // if (method == "delete")
    //     return new DeleteRequest();
    return 0;
}

//--------------------------GET--------------------------
bool GetHeadRequest::validate(std::string &err) const
{
    if (!body.empty())
    {
        err = "GET/HEAD request should not have a body";
        return false;
    }
    return true;
}

GetHeadRequest::GetHeadRequest(const RequestContext &ctx) : HttpRequest(ctx)
{
}

GetHeadRequest::~GetHeadRequest()
{
}

void GetHeadRequest::handle(HttpResponse &res)
{
    bool includeBody = (method == "GET");
    handleGetOrHead(res, includeBody);
}

void HttpRequest::handleGetOrHead(HttpResponse &res, bool includeBody)
{
    // Check the actual method being requested
    if ((method == "GET" && !_ctx.isMethodAllowed("GET")) || (method == "HEAD" && !_ctx.isMethodAllowed("HEAD")))
    {
        res.setErrorFromContext(405, _ctx);
        return;
    }

    std::string fullPath = _ctx.getFullPath(path);
    struct stat fileStat;

    if (stat(fullPath.c_str(), &fileStat) != 0)
    {
        res.setErrorFromContext(404, _ctx);
        return;
    }

    if (S_ISDIR(fileStat.st_mode))
    {
        bool found = false;
        const std::vector<std::string> &indexFiles = _ctx.getIndexFiles();
        for (size_t i = 0; i < indexFiles.size(); i++)
        {
            std::string indexPath = fullPath;
            if (fullPath.empty() || fullPath[fullPath.size() - 1] != '/')
                indexPath += '/';
            indexPath += indexFiles[i];

            if (stat(indexPath.c_str(), &fileStat) == 0)
            {
                fullPath = indexPath;
                found = true;
                break;
            }
        }

        if (!found)
        {
            if (_ctx.getAutoIndex())
            {
                res.setStatus(200, "OK");
                res.setHeader("Content-Type", "text/html");
                if (includeBody)
                {
                    // TODO: generate directory listing HTML
                }
                return;
            }
            res.setErrorFromContext(404, _ctx);
            return;
        }
    }

    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        res.setErrorFromContext(403, _ctx);
        return;
    }

    std::ostringstream content;
    if (includeBody)
        content << file.rdbuf();

    std::ostringstream lenStream;
    lenStream << fileStat.st_size;

    res.setStatus(200, "OK");
    res.setHeader("Content-Length", lenStream.str());
    res.setHeader("Content-Type", getMimeType(fullPath));
    if (includeBody)
        res.setBody(content.str());
}

//--------------------------POST--------------------------
// Basil : What is the point of this Validation?? POST can have an empty Body.
bool PostRequest::validate(std::string &err) const
{
    // POST can have empty body for some CGI scenarios
    if (contentLength() == 0)
    {
        err = "Missing body in POST request";
        return false;
    }
    return true;
}

PostRequest::PostRequest(const RequestContext &ctx) : HttpRequest(ctx)
{
}

PostRequest::~PostRequest()
{
}

bool PostRequest::isPathSafe(const std::string &path) const
{
    if (path.find("..") != std::string::npos)
        return false;
    return true;
}

void PostRequest::handle(HttpResponse &res)
{
    if (!_ctx.isMethodAllowed("POST"))
    {
        res.setErrorFromContext(405, _ctx);
        return;
    }

    // 1. Determine upload directory
    std::string uploadDir;
    if (_ctx.location && !_ctx.location->getUploadDir().empty())
    {
        uploadDir = _ctx.location->getUploadDir();
    }
    else
    {
        uploadDir = _ctx.server.getRoot(); // fallback to server root
    }

    // 2. Ensure directory ends with '/'
    if (!uploadDir.empty() && uploadDir[uploadDir.size() - 1] != '/')
        uploadDir += '/';

    // 3. Build full file path
    std::string filename = extractFileName(path); // utility to get last part of path

    // Check if filename is empty (e.g., path was just "/upload" or "/")
    if (filename.empty())
    {
        // Generate a default filename with timestamp
        std::time_t now = std::time(0);
        std::ostringstream oss;
        oss << "upload_" << now << ".txt";
        filename = oss.str();
    }

    std::string fullPath = uploadDir + filename;

    // 4. Path traversal check
    if (!isPathSafe(fullPath))
    {
        res.setErrorFromContext(403, _ctx);
        return;
    }

    // 5. Attempt to write the body
    std::ofstream outFile(fullPath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        res.setErrorFromContext(500, _ctx);
        return;
    }
    outFile << body;
    outFile.close();

    // 6. Send response
    res.setStatus(201, "Created");
    res.setHeader("Content-Length", "0");
    res.setHeader("Content-Type", "text/plain");
}
