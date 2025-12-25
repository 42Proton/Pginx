#include "requestContext.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

// Server = the entire building's rules (global settings)
// location = specific room rules that override building rules

// server vs location
/*
Server (Global Configuration):
    - Scope: Applies to the entire web server
    - Purpose: Default settings for all requests
    - Example: "By default, all rooms in this building allow 50 people max"
*/
/*
Location (Specific Path Configuration):
    - Scope: Applies only to specific URL paths
    - Purpose: Override server settings for specific paths
    - Example: "But the conference room allows 100 people max"
*/

RequestContext::RequestContext(const Server& srv, const LocationConfig* loc)
    : server(srv), location(loc), rootDir("") {
  rootDir = server.getRoot();
  // Only use location's root if it's explicitly set (not the default)
  if (location && !location->getRoot().empty() &&
      location->getRoot() != DEFAULT_ROOT_PATH)
    rootDir = location->getRoot();
}

// index files are the default files that a web server serves when someone
// requests a dir (instead of a specific file)
const std::vector<std::string>& RequestContext::getIndexFiles() const {
  if (location && !location->getIndexFiles().empty())
    return location->getIndexFiles();
  return server.getIndexFiles();
}

size_t RequestContext::getClientMaxBodySize() const {
  if (location)
    return location->getClientMaxBodySize();
  return server.getClientMaxBodySize();
}

bool RequestContext::getAutoIndex() const {
  if (location)
    return location->getAutoIndex();
  return server.getAutoIndex();
}

bool RequestContext::isMethodAllowed(const std::string& method) const {
  if (location)
    return location->isMethodAllowed(method);
  return (method == "GET" || method == "HEAD" || method == "POST" ||
          method == "PUT" || method == "DELETE" || method == "PATCH");
}

// converts a relative URL path (from an HTTP request) into an absolute file
// system path that your server can use to find the actual file.
std::string RequestContext::getFullPath(const std::string& requestPath) const {
  std::string fullPath = "";

  // If empty, return rootDir
  if (requestPath.empty())
    return rootDir;

  // Check if requestPath is absolute (starts with '/')
  if (!requestPath.empty() && requestPath[0] == '/') {
    // Treat absolute paths as relative to rootDir for security
    fullPath = rootDir;
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
      fullPath += '/';

    // Remove leading slash from requestPath
    fullPath += requestPath.substr(1);
  } else {
    // Relative path, just combine with rootDir
    fullPath = rootDir;
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
      fullPath += '/';

    fullPath += requestPath;
  }

  return fullPath;
}

const std::string* RequestContext::getErrorPage(const u_int16_t code) const {
  // Try location first, then server
  if (location) {
    const std::string* errorPage = location->getErrorPage(code);
    if (errorPage)
      return errorPage;
  }
  return server.getErrorPage(code);
}

std::string RequestContext::getErrorPageContent(u_int16_t code) const {
  const std::string* pagePath = getErrorPage(code);
  if (!pagePath || pagePath->empty()) {
    throw std::runtime_error("No error page configured for code");
  }

  std::string fullErrorPath = getFullPath(*pagePath);
  std::ifstream file(fullErrorPath.c_str(), std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open error page file: " +
                             fullErrorPath);
  }

  std::string content;
  char buffer[4096];
  while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
    content.append(buffer, file.gcount());
  }

  if (file.bad()) {
    throw std::runtime_error("Failed to read error page file: " +
                             fullErrorPath);
  }

  return content;
}

bool RequestContext::hasReturn() const {
  if (location && location->hasReturn())
    return true;
  return server.hasReturn();
}

const std::pair<u_int16_t, std::string>& RequestContext::getReturnData() const {
  if (location && location->hasReturn())
    return location->getReturnData();
  return server.getReturnData();
}
