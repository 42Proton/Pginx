#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>
class Server : public BaseBlock
{
  private:
    std::vector<u_int16_t> _ports;
    std::string _serverName;

  public:
    Server();
    const std::vector<u_int16_t> &getPorts() const;
    const std::string &getServerName() const;
    void setPorts(const std::vector<u_int16_t> &ports);
    void setServerName(const std::string &serverName);
};

#endif