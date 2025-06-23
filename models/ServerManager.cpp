#include "ServerManager.hpp"

// Constructors
ServerManager::ServerManager()
{
}

ServerManager::ServerManager(const ServerManager &copy)
{
    (void)copy;
}

// Destructor
ServerManager::~ServerManager()
{
}

// Operators
ServerManager &ServerManager::operator=(const ServerManager &assign)
{
    (void)assign;
    return *this;
}
