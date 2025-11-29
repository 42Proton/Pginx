#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <BaseBlock.hpp>
#include <vector>

enum MatchType {
  PREFIX,           // Default: location /path
  EXACT,            // Exact: location = /path
  REGEX_CASE,       // Case-sensitive regex: location ~ pattern
  REGEX_ICASE,      // Case-insensitive regex: location ~* pattern
  PRIORITY_PREFIX,  // Priority prefix: location ^~ /path
  NAMED             // Named location: location @name
};

class LocationConfig : public BaseBlock {
 private:
  std::string _path;
  MatchType _matchType;
  std::vector<std::string> _methods;
  std::string _uploadDir;
  bool _cgi_enabled;
  bool _chunked_transfer_encoding;
  std::map<std::string, std::string> _cgiPassMap;

 public:
  LocationConfig();
  LocationConfig(const std::string& path);
  LocationConfig(const std::string& path, MatchType matchType);
  LocationConfig(const LocationConfig& obj);
  ~LocationConfig();

  // Setters
  void setPath(const std::string& path);
  void setMatchType(MatchType matchType);
  void addMethod(const std::string& method);
  void setMethods(const std::vector<std::string>& methods);
  void setUploadDir(const std::string& dir);
  void setCgiEnabled(bool enabled);
  bool isCgiEnabled() const;
  void setTransferEncoding(bool enabled);
  void setCgiPassMapping(const std::string& extension, const std::string& interpreterPath);

  // Getters
  const std::string& getPath() const;
  MatchType getMatchType() const;
  const std::vector<std::string>& getMethods() const;
  bool isMethodAllowed(const std::string& method) const;
  const std::string& getUploadDir() const;
};

#endif