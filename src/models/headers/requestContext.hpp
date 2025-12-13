#ifndef REQUESTCONTEXT_HPP
#define REQUESTCONTEXT_HPP

#include <string>
#include "Server.hpp"
#include "LocationConfig.hpp"

class RequestContext {
  public:
    const Server &server;
    const LocationConfig *location;
    std::string rootDir;

    
    RequestContext(const Server &srv, const LocationConfig *loc);

    // Configuration getters with location/server fallback
    const std::vector<std::string> &getIndexFiles() const;
    size_t getClientMaxBodySize() const;
    bool getAutoIndex() const;
    const std::string *getErrorPage(const u_int16_t code) const;
    
    
    // Method validation
    bool isMethodAllowed(const std::string &method) const;
    
    // Helper methods
    std::string getFullPath(const std::string &requestPath) const;
    std::string getErrorPageContent(u_int16_t code) const;
};

#endif
