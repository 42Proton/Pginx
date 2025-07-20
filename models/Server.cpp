#include <Server.hpp>

Server::Server():
    BaseBlock(),
    _isDefault(false),
    _ports(),
    _serverName()
{}

bool Server::getIsDefault() const
{
    return this->_isDefault;
}

const std::vector<u_int16_t>& Server::getPorts() const
{
    return this->_ports;
}

const std::string& Server::getServerName() const
{
    return this->_serverName;
}

void Server::setIsDefault(bool isDefault)
{
    this->_isDefault = isDefault;
}

void Server::setPorts(std::vector<u_int16_t>& ports)
{
    this->_ports = ports;
}

void Server::setServerName(std::string& serverName)
{
    this->_serverName = serverName;
}