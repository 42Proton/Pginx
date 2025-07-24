#include <Server.hpp>

Server::Server() : BaseBlock()
{
    this->_serverNames.push_back("");
}

bool Server::validatePort(u_int16_t port) const
{
    // I'll leave this as a placeholder for now.
    // We might add the ability to check if the port is already in use or not.
    // For now, we will just check if the port is within the valid range.
    return port;
}

bool Server::validateAddress(const std::string &addr) const
{
    if (addr.empty())
        return true;
    std::vector<std::string> parts = split(addr, '.');
    if (parts.size() != 4)
        return true;
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (parts[i].empty() || parts[i].length() > 3)
            return true;
        for (size_t j = 0; j < parts[i].length(); ++j)
        {
            if (!isdigit(parts[i][j]))
                return true;
            int partValue = std::strtol(parts[i].c_str(), NULL, 10);
            if (partValue < 0 || partValue > 255)
                return true;
        }
    }
    return false;
}

const ListenCtx &Server::getListen() const
{
    return this->_listen;
}

const std::vector<std::string> &Server::getServerNames() const
{
    return this->_serverNames;
}

void Server::setListen(u_int16_t port, const std::string &addr)
{
    if (validatePort(port) || validateAddress(addr))
        throw CommonExceptions::InititalaizingException();
    this->_listen.addr = addr;
    this->_listen.port = port;
}

void Server::insertServerNames(const std::string &serverName)
{
    if (serverName.empty())
        return;
    if (this->_serverNames.size() == 1 && this->_serverNames[0].empty())
    {
        this->_serverNames[0] = serverName;
        return;
    }
    if (std::find(this->_serverNames.begin(), this->_serverNames.end(), serverName) != this->_serverNames.end())
        return;

    this->_serverNames.push_back(serverName);
}
