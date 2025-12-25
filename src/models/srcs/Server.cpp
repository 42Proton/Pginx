#include <Server.hpp>

/*
    You don't need to really understand these things below, refer to the Header
   file
*/

Server::Server() : BaseBlock(), _root("") {
  this->_serverNames.push_back("");
  setRoot();
}

bool Server::validateAddress(const std::string& addr) const {
  if (addr.empty())
    return true;
  std::vector<std::string> parts = split(addr, '.');
  if (parts.size() != 4)
    return true;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (parts[i].empty() || parts[i].length() > 3)
      return true;
    for (size_t j = 0; j < parts[i].length(); ++j) {
      if (!isdigit(parts[i][j]))
        return true;
      int partValue = std::strtol(parts[i].c_str(), NULL, 10);
      if (partValue < 0 || partValue > 255)
        return true;
    }
  }
  return false;
}

const std::vector<ListenCtx>& Server::getListens() const {
  return this->_listens;
}

const std::vector<std::string>& Server::getServerNames() const {
  return this->_serverNames;
}

void Server::insertListen(u_int16_t port, const std::string& addr) {
  if (validateAddress(addr))
    throw CommonExceptions::InititalaizingException();
  ListenCtx newListen;
  newListen.addr = addr;
  newListen.port = port;
  if (std::find(this->_listens.begin(), this->_listens.end(), newListen) !=
      this->_listens.end())
    return;
  this->_listens.push_back(newListen);
}

void Server::insertServerNames(const std::string& serverName) {
  if (serverName.empty())
    return;
  if (this->_serverNames.size() == 1 && this->_serverNames[0].empty()) {
    this->_serverNames[0] = serverName;
    return;
  }
  if (std::find(this->_serverNames.begin(), this->_serverNames.end(),
                serverName) != this->_serverNames.end())
    return;

  this->_serverNames.push_back(serverName);
}

void Server::setRoot(const std::string& root) {
  if (root.empty())
    throw CommonExceptions::InititalaizingException();
  if (root[root.length() - 1] != '/') {
    this->_root = root + '/';
    return;
  }
  struct stat st;
  if (stat(root.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
    throw CommonExceptions::OpenFileException();
  }
  if (access(root.c_str(), R_OK) != 0) {
    throw CommonExceptions::OpenFileException();
  }

  this->_root = root;
}

const std::string& Server::getRoot() const {
  return this->_root;
}

void Server::setIndexFiles(const std::vector<std::string>& indexFiles) {
  this->insertIndex(indexFiles);
}

const std::vector<std::string>& Server::getIndexFiles() const {
  return this->_indexFiles;
}

void Server::addLocation(const LocationConfig& location) {
  this->_locations.push_back(location);
}

const std::vector<LocationConfig>& Server::getLocations() const {
  return this->_locations;
}

const std::string& Server::getMatchingServerName(const std::string &hostHeader) const {
  // Extract hostname from Host header (remove port if present)
  std::string hostname = hostHeader;
  size_t colonPos = hostname.find(':');
  if (colonPos != std::string::npos) {
    hostname = hostname.substr(0, colonPos);
  }

  // Try to find exact match in server_name list
  for (std::vector<std::string>::const_iterator it = _serverNames.begin();
       it != _serverNames.end(); ++it) {
    if (*it == hostname) {
      return *it;
    }
  }

  // If no match found, return first server name or "localhost"
  if (!_serverNames.empty()) {
    return _serverNames[0];
  }
  
  static const std::string defaultName = "localhost";
  return defaultName;
}

const LocationConfig* Server::findLocation(const std::string& path) const {
  // Find the most specific location that matches the path
  const LocationConfig* bestMatch = NULL;
  size_t longestMatch = 0;

  for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
       it != _locations.end(); ++it) {
    const std::string& locationPath = it->getPath();
    if (path.find(locationPath) == 0 && locationPath.length() > longestMatch) {
      bestMatch = &(*it);
      longestMatch = locationPath.length();
    }
  }

  return bestMatch;
}

u_int16_t Server::getServerPort(std::string server) const {
    for (std::vector<ListenCtx>::const_iterator it = _listens.begin(); it != _listens.end(); ++it) {
        if (server == "" || std::find(_serverNames.begin(), _serverNames.end(), server) != _serverNames.end()) {
            return it->port;
        }
    }
    return 80;
}

const std::string& Server::getServerAddr(std::string server) const {
    for (std::vector<ListenCtx>::const_iterator it = _listens.begin(); it != _listens.end(); ++it) {
        if (server == "" || std::find(_serverNames.begin(), _serverNames.end(), server) != _serverNames.end()) {
            return it->addr;
        }
    }
    static const std::string defaultAddr = "0.0.0.0";
    return defaultAddr;
}

void Server::enableCgi(bool enabled) {
    setCgiEnabled(enabled);
}

bool Server::isCgiEnabled() const {
    return BaseBlock::isCgiEnabled();
}