#include <Container.hpp>

Container::Container()
{
}

Container::~Container()
{
}

void Container::insertServer(const Server &server)
{
    this->_servers.push_back(server);
}

const std::vector<Server> &Container::getServers() const
{
    return this->_servers;
}