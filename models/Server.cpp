#include <Server.hpp>

bool Server::getIsDefault() const
{
    return _isdefault;
}

void Server::setIsDefault()
{
    _isdefault = true;
}
std::vector<u_int16_t> Server::getPorts() const
{
    return _PORTS;
}
std::string Server::getServerName() const
{
    return _serverName;
}
