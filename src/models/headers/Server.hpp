#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>
#include <LocationConfig.hpp>

struct ListenCtx {
  u_int16_t port;
  std::string addr;
  
  bool operator==(const ListenCtx &other) const {
      return this->port == other.port && this->addr == other.addr;
  }
  bool operator!=(const ListenCtx &other) const {
    return !(*this == other);
  }
  
};


class Server : public BaseBlock {
  private:
    std::vector<ListenCtx> _listens;
    std::vector<std::string> _serverNames;
    std::string _root;
    std::vector<LocationConfig> _locations;

    bool validateAddress(const std::string &addr) const;

  public:
    Server();
    ~Server() {};

    const std::vector<ListenCtx> &getListens() const;
    u_int16_t getServerPort(std::string server) const;
    const std::vector<std::string> &getServerNames() const;
    const std::string &getMatchingServerName(const std::string &hostHeader) const;
    void insertListen(u_int16_t port = 80, const std::string &addr = "0.0.0.0");
    void insertServerNames(const std::string &serverName);
    void setRoot(const std::string &root = "www/");
    const std::string &getRoot() const;
    const std::string &getServerAddr(std::string server) const;
    void setIndexFiles(const std::vector<std::string> &indexFiles);
    const std::vector<std::string> &getIndexFiles() const;

    // Location management
    void addLocation(const LocationConfig &location);
    const std::vector<LocationConfig> &getLocations() const;
    const LocationConfig *findLocation(const std::string &path) const;
};

#endif