#ifndef SERVER_HPP
#define SERVER_HPP

#include <AServer.hpp>

class Server : public AServer
{
  private:
    bool _isdefault;
    std::vector<u_int16_t> _PORTS;
    std::string _serverName;
    std::string _root;

  public:
    Server();
    bool getIsDefault() const;
    void setIsDefault();
    std::vector<u_int16_t> getPorts() const;
    std::string getServerName() const;
    std::string getRoot() const;
};

#endif