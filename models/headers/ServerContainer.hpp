#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <BaseBlock.hpp>
#include <Server.hpp>
#include <utils.hpp>

class ServerContainer : public BaseBlock
{
  private:
    std::vector<Server> _servers;

  public:
    ServerContainer();
    ~ServerContainer();
    void insertServer(const Server& server);
};

#endif