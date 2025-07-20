#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <Server.hpp>
#include <utils.hpp>

class ServerContainer
{
    private:
        std::vector<Server> _servers;
    public:
        ServerContainer();
        ~ServerContainer();
        void setServers(const std::vector<Server>& servers);
        const std::vector<Server>& getServers() const;
};

#endif