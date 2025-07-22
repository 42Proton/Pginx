#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>

class Server : public BaseBlock
{
  private:
    bool _isDefault;
    std::vector<u_int16_t> _ports;
    std::string _serverName;

  public:
    Server();
    bool getIsDefault() const;
    const std::vector<u_int16_t> &getPorts() const;
    const std::string &getServerName() const;
    void setIsDefault(bool isDefault);
    void setPorts(std::vector<u_int16_t> &ports);
    void setServerName(std::string &serverName);
};

#endif