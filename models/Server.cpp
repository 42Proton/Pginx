#include <Server.hpp>

std::string Server::_defaultServerName = "";

Server::Server() : BaseBlock(), _ports(), _serverName()
{
    if (_defaultServerName.empty())
    {
        _defaultServerName = "default_server";
    }
}

bool Server::isDefault() const
{
    return this->_defaultServerName == this->_serverName;
}

const std::vector<u_int16_t> &Server::getPorts() const
{
    return this->_ports;
}

const std::string &Server::getServerName() const
{
    return this->_serverName;
}

void Server::setIsDefault(const std::string &defaultServerName)
{
    this->_defaultServerName = defaultServerName;
}

void Server::setPorts(const std::vector<u_int16_t> &ports)
{
    this->_ports = ports;
}

void Server::setServerName(const std::string &serverName)
{
    this->_serverName = serverName;
}