#ifndef REQUESTCONTEXT_HPP
#define REQUESTCONTEXT_HPP

#include <string>
#include "LocationConfig.hpp"
#include "Server.hpp"

class RequestContext {
 public:
  const Server& server;
  const LocationConfig* location;
  std::string rootDir;

  RequestContext(const Server& srv, const LocationConfig* loc);
  const std::vector<std::string>& getIndexFiles() const;
  size_t getClientMaxBodySize() const;
  bool getAutoIndex() const;
  const std::string* getErrorPage(const u_int16_t code) const;
  bool isMethodAllowed(const std::string& method) const;
  std::string getFullPath(const std::string& requestPath) const;
  std::string getErrorPageContent(u_int16_t code) const;
  bool hasReturn() const;
  const std::pair<u_int16_t, std::string>& getReturnData() const;
};

#endif
