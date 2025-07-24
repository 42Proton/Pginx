#include <ServerContainer.hpp>

ServerContainer::ServerContainer() {}

ServerContainer::~ServerContainer() {}

void ServerContainer::setServers(const std::vector<Server>& servers)
{
	this->_servers = servers;
}

const std::vector<Server>& ServerContainer::getServers() const
{
	return this->_servers;
}