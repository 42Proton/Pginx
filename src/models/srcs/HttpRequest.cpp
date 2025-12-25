#include "HttpRequest.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include "CgiHandle.hpp"
#include "HttpResponse.hpp"
#include "HttpUtils.hpp"

HttpRequest::HttpRequest(const RequestContext& ctx) : _ctx(ctx) {}

// Copy assignment operator (private - not meant to be used)
// Note: _ctx cannot be reassigned as it's a const reference
HttpRequest& HttpRequest::operator=(const HttpRequest& other) {
  if (this != &other) {
    // _ctx is a const reference and cannot be reassigned
    // Only copy non-const members
    method = other.method;
    path = other.path;
    version = other.version;
    headers = other.headers;
    body = other.body;
    query = other.query;
    enabledCgi = other.enabledCgi;
  }
  return *this;
}

HttpRequest::~HttpRequest() {}

bool HttpRequest::isCgiEnabledForRequest() const {
  // Location-level setting overrides server-level setting
  // If no location matches, use server-level setting
  // Note: Locations inherit from server if not explicitly set during parsing
  if (_ctx.location) {
    return _ctx.location->isCgiEnabled();
  }
  return _ctx.server.isCgiEnabled();
}

const std::string& HttpRequest::getMethod() const {
  return method;
}

const std::string& HttpRequest::getPath() const {
  return path;
}

const std::string& HttpRequest::getVersion() const {
  return version;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
  return headers;
}

const std::string& HttpRequest::getBody() const {
  return body;
}

const std::map<std::string, std::string>& HttpRequest::getQuery() const {
  return query;
}

void HttpRequest::setMethod(const std::string& m) {
  method = m;
}

void HttpRequest::setPath(const std::string& p) {
  path = p;
}

void HttpRequest::setVersion(const std::string& v) {
  version = v;
}

void HttpRequest::setEnabledCgi(bool enabled) {
  enabledCgi = enabled;
}

void HttpRequest::addHeader(const std::string& k, const std::string& v) {
  headers[k] = v;
}

void HttpRequest::appendBody(const std::string& data) {
  body.append(data);
}

void HttpRequest::setQuery(const std::map<std::string, std::string>& q) {
  query = q;
}

bool HttpRequest::validate(std::string& err) const {
  (void)err;
  return true;
}

bool HttpRequest::isChunked() const {
  std::map<std::string, std::string>::const_iterator it =
      headers.find("transfer-encoding");
  if (it == headers.end())
    return false;

  std::string value = toLowerStr(it->second);
  return (value.find("chunked") != std::string::npos);
}

size_t HttpRequest::contentLength() const {
  std::map<std::string, std::string>::const_iterator it =
      headers.find("content-length");
  if (it == headers.end())
    return 0;
  return safeAtoi(it->second);
}

void HttpRequest::parseQuery(const std::string& target, std::string& cleanPath, std::map<std::string, std::string>& outQuery) {
  size_t qpos = target.find('?');
  if (qpos == std::string::npos) {
    cleanPath = target;
    return;
  }
  cleanPath = target.substr(0, qpos);
  std::string qstr = target.substr(qpos + 1);

  size_t start = 0;
  while (start < qstr.size()) {
    size_t eq = qstr.find('=', start);
    size_t amp = qstr.find('&', start);
    std::string key, val;

    if (eq == std::string::npos || (amp != std::string::npos && amp < eq)) {
      key = urlDecode(qstr.substr(
          start, (amp == std::string::npos ? qstr.size() : amp) - start));
      val = "";
    } else {
      key = urlDecode(qstr.substr(start, eq - start));
      val = urlDecode(qstr.substr(
          eq + 1, (amp == std::string::npos ? qstr.size() : amp) - eq - 1));
    }

    if (!key.empty())
      outQuery[key] = val;
    if (amp == std::string::npos)
      break;
    start = amp + 1;
  }
}

bool HttpRequest::parseHeaderLine(const std::string& line, std::string& k, std::string& v) {
  size_t colon = line.find(':');
  if (colon == std::string::npos)
    return false;
  k = toLowerStr(trim(line.substr(0, colon)));
  v = trim(line.substr(colon + 1));
  return true;
}

HttpRequest* makeRequestByMethod(const std::string& method, const RequestContext& ctx) {
  if (method == "GET" || method == "HEAD")
    return new GetHeadRequest(ctx);
  if (method == "POST")
    return new PostRequest(ctx);
  if (method == "DELETE")
    return new DeleteRequest(ctx);
  return 0;
}

//--------------------------GET--------------------------
bool GetHeadRequest::validate(std::string& err) const {
  if (!body.empty()) {
    err = "GET/HEAD request should not have a body";
    return false;
  }
  return true;
}

GetHeadRequest::GetHeadRequest(const RequestContext& ctx) : HttpRequest(ctx) {}

GetHeadRequest::~GetHeadRequest() {}

void GetHeadRequest::handle(HttpResponse& res, sockaddr_in& clientAddr, int epollFd) {
  bool includeBody = (method == "GET");
  handleGetOrHead(res, includeBody, clientAddr, epollFd);
}

void HttpRequest::handleGetOrHead(HttpResponse& res, bool includeBody, sockaddr_in& clientAddr, int epollFd) {
  if (!_ctx.isMethodAllowed(method)) {
    res.setErrorFromContext(405, _ctx);
    return;
  }

  std::string fullPath = _ctx.getFullPath(path);
  struct stat fileStat;
  std::memset(&fileStat, 0, sizeof(fileStat));

  if (stat(fullPath.c_str(), &fileStat) != 0) {
    res.setErrorFromContext(404, _ctx);
    return;
  }

  // Check if CGI is enabled (location overrides server setting) and file is not a directory
  if (isCgiEnabledForRequest() && !S_ISDIR(fileStat.st_mode)) {
    // Handle CGI requests
    CgiHandle cgiHandler;
    std::string scriptPath = _ctx.getFullPath(path);
    if (scriptPath.empty()) {
      res.setErrorFromContext(500, _ctx);
      return;
    }
    struct stat scriptStat;
    std::memset(&scriptStat, 0, sizeof(scriptStat));
    if (stat(scriptPath.c_str(), &scriptStat) != 0 ||
        !(scriptStat.st_mode & S_IXUSR)) {
      res.setErrorFromContext(403, _ctx);
      return;
    }
    cgiHandler.buildCgiScript(scriptPath, _ctx, res, *this, clientAddr,
                              epollFd);
    return;
  }

  if (S_ISDIR(fileStat.st_mode)) {
    bool found = false;
    const std::vector<std::string>& indexFiles = _ctx.getIndexFiles();
    for (size_t i = 0; i < indexFiles.size(); i++) {
      std::string indexPath = fullPath;
      if (fullPath.empty() || fullPath[fullPath.size() - 1] != '/')
        indexPath += '/';
      indexPath += indexFiles[i];

      if (stat(indexPath.c_str(), &fileStat) == 0) {
        fullPath = indexPath;
        found = true;
        break;
      }
    }

    if (!found) {
      std::cerr << "is enabled autoindex: " << _ctx.getAutoIndex() << "\n";
      if (_ctx.getAutoIndex()) {
        std::string page = generateAutoIndexPage(fullPath, path);
        if (includeBody)
          res.setBody(page);
        std::ostringstream lenStream;
        lenStream << page.size();
        res.setHeader("Content-Length", lenStream.str());
        res.setStatus(200, "OK");
        res.setHeader("Content-Type", "text/html");
        return;
      }
      res.setErrorFromContext(404, _ctx);
      return;
    }
  }

  std::ifstream file(fullPath.c_str(), std::ios::binary);
  if (!file.is_open()) {
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
bool PostRequest::validate(std::string& err) const {
  // For chunked requests, body might exist even without Content-Length
  // initially After un-chunking, the parser should have set Content-Length Also
  // allow requests with actual body content even if Content-Length is 0
  if (contentLength() == 0 && body.empty()) {
    err = "Missing body in POST request";
    return false;
  }
  return true;
}

PostRequest::PostRequest(const RequestContext& ctx) : HttpRequest(ctx) {}

PostRequest::~PostRequest() {}

bool PostRequest::isPathSafe(const std::string& path) const {
  if (path.find("..") != std::string::npos)
    return false;
  return true;
}

void PostRequest::handle(HttpResponse& res, sockaddr_in& clientAddr, int epollFd) {
  if (!_ctx.isMethodAllowed("POST")) {
    res.setErrorFromContext(405, _ctx);
    return;
  }

  // Check if CGI is enabled (location overrides server setting)
  if (isCgiEnabledForRequest()) {
    // Handle CGI requests
    CgiHandle cgiHandler;
    std::string scriptPath = _ctx.getFullPath(path);
    if (scriptPath.empty()) {
      res.setErrorFromContext(500, _ctx);
      return;
    }
    struct stat scriptStat;
    std::memset(&scriptStat, 0, sizeof(scriptStat));  // Initialize to zero
    if (stat(scriptPath.c_str(), &scriptStat) != 0 ||
        !(scriptStat.st_mode & S_IXUSR)) {
      res.setErrorFromContext(403, _ctx);
      return;
    }
    // Execute the CGI script
    cgiHandler.buildCgiScript(scriptPath, _ctx, res, *this, clientAddr,
                              epollFd);
    return;
  }
  std::string uploadDir;
  if (_ctx.location && !_ctx.location->getUploadDir().empty()) {
    uploadDir = _ctx.location->getUploadDir();
  } else {
    uploadDir = _ctx.server.getRoot();
  }

  if (!uploadDir.empty() && uploadDir[uploadDir.size() - 1] != '/')
    uploadDir += '/';

  std::string filename = extractFileName(path);
  if (filename.empty()) {
    std::time_t now = std::time(0);
    std::ostringstream oss;
    oss << "upload_" << now << ".txt";
    filename = oss.str();
  }

  std::string fullPath = uploadDir + filename;
  if (!isPathSafe(fullPath)) {
    res.setErrorFromContext(403, _ctx);
    return;
  }

  bool createdNew = true;
  std::ifstream checkFile(fullPath.c_str());
  if (checkFile.good()) {
    createdNew = false;
  }
  checkFile.close();

  std::ofstream outFile(fullPath.c_str(), std::ios::out | std::ios::binary);
  if (!outFile.is_open()) {
    res.setErrorFromContext(500, _ctx);
    return;
  }
  outFile << body;
  outFile.close();

  if (createdNew) {
    res.setStatus(201, "Created");
    res.setHeader("Content-Length", "0");
    res.setHeader("Content-Type", "text/plain");
  } else {
    std::ostringstream msg;
    msg << "File updated successfully: " << filename << "\n";
    std::string msgStr = msg.str();

    std::ostringstream len;
    len << msgStr.size();

    res.setStatus(200, "OK");
    res.setHeader("Content-Length", len.str());
    res.setHeader("Content-Type", "text/plain");
    res.setBody(msgStr);
  }
}

DeleteRequest::DeleteRequest(const RequestContext& ctx) : HttpRequest(ctx) {}

DeleteRequest::~DeleteRequest() {}

bool DeleteRequest::validate(std::string& err) const {
  if (!body.empty()) {
    err = "DELETE request should not have a body";
    return false;
  }
  return true;
}

bool DeleteRequest::isPathSafe(const std::string& fullPath) const {
  if (fullPath.find("..") != std::string::npos)
    return false;

  std::string rootDir = _ctx.rootDir;
  if (fullPath.find(rootDir) != 0)
    return false;

  return true;
}

void DeleteRequest::handle(HttpResponse& res, sockaddr_in& clientAddr, int epollFd) {
  (void)epollFd;
  (void)clientAddr;
  if (!_ctx.isMethodAllowed("DELETE")) {
    res.setErrorFromContext(405, _ctx);
    return;
  }

  std::string fullPath = _ctx.getFullPath(path);

  if (!isPathSafe(fullPath)) {
    res.setErrorFromContext(403, _ctx);
    return;
  }

  struct stat fileStat;
  std::memset(&fileStat, 0, sizeof(fileStat));  // Initialize to zero
  if (stat(fullPath.c_str(), &fileStat) != 0) {
    res.setErrorFromContext(404, _ctx);
    return;
  }

  int result;
  if (S_ISDIR(fileStat.st_mode)) {
    // Nginx-style: only delete empty directories, return 409 Conflict if not
    // empty
    result = rmdir(fullPath.c_str());

    if (result != 0 && errno == ENOTEMPTY) {
      res.setStatus(409, "Conflict");
      res.setHeader("Content-Type", "text/plain");
      std::string body = "Cannot delete non-empty directory";
      std::ostringstream lenStream;
      lenStream << body.length();
      res.setHeader("Content-Length", lenStream.str());
      res.setBody(body);
      return;
    }
  } else {
    result = remove(fullPath.c_str());
  }

  if (result != 0) {
    if (errno == EACCES || errno == EPERM) {
      res.setErrorFromContext(403, _ctx);
    } else {
      res.setErrorFromContext(500, _ctx);
    }
    return;
  }
  res.setStatus(204, "No Content");
  res.setHeader("Content-Length", "0");
}
