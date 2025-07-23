#include <Server.hpp>

Server::Server() : BaseBlock(), _ports(), _serverName()
{
}

const std::vector<u_int16_t> &Server::getPorts() const
{
    return this->_ports;
}

const std::string &Server::getServerName() const
{
    return this->_serverName;
}

void Server::setPorts(const std::vector<u_int16_t> &ports)
{
    this->_ports = ports;
}

void Server::setServerName(const std::string &serverName)
{
    this->_serverName = serverName;
}