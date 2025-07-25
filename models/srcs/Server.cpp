#include <Server.hpp>

Server::Server() : BaseBlock()
{
    this->_serverNames.push_back("");
    setRoot();
    insertListen();
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

const std::vector<ListenCtx> &Server::getListens() const
{
    return this->_listens;
}

const std::vector<std::string> &Server::getServerNames() const
{
    return this->_serverNames;
}

void Server::insertListen(u_int16_t port, const std::string &addr)
{
    if (validateAddress(addr))
        throw CommonExceptions::InititalaizingException();
    ListenCtx newListen;
    newListen.addr = addr;
    newListen.port = port;
    if (std::find(this->_listens.begin(), this->_listens.end(), newListen) != this->_listens.end())
        return;
    this->_listens.push_back(newListen);
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

void Server::setRoot(const std::string &root)
{
    if (root.empty())
        throw CommonExceptions::InititalaizingException();
    if (root[root.length() - 1] != '/')
    {
        this->_root = root + '/';
        return;
    }
    struct stat st;
    if (stat(root.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
    {
        throw CommonExceptions::OpenFileException();
    }
    if (access(root.c_str(), R_OK) != 0)
    {
        throw CommonExceptions::OpenFileException();
    }

    this->_root = root;
}

const std::string &Server::getRoot() const
{
    return this->_root;
}