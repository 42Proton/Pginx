#ifndef SERVER_HPP
#define SERVER_HPP

#include <utils.hpp>

class Server
{
  private:
    bool _isdefault;
    std::vector<u_int16_t> _PORTS;
    std::string _serverName;
    std::ifstream _errorPages;
};

#endif