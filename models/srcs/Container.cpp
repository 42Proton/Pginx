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