#ifndef SERVER_HPP
#define SERVER_HPP

#include <AServer.hpp>

class Server : public AServer
{
  private:
    bool _isdefault;
    std::vector<u_int16_t> _PORTS;
    std::string _serverName;
    Server();

  public:
    Server(std::vector<u_int16_t> ports, std::string serverName, bool isDefault = false);
    bool getIsDefault() const;
    void setIsDefault();
    std::vector<u_int16_t> getPorts() const;
    std::string getServerName() const;
};

#endif