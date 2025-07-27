#include <ServerContainer.hpp>

ServerContainer::ServerContainer() : AccessPermission() {}

ServerContainer::~ServerContainer() {}

void ServerContainer::insertServer(const Server& server)
{
	this->_servers.push_back(server);
}