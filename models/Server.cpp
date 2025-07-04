#include <Server.hpp>

Server::Server(std::vector<u_int16_t> ports, std::string serverName, bool isDefault = false)
    : _PORTS(ports), _serverName(serverName), _isdefault(isDefault)
{
}

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
