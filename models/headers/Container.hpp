#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <BaseBlock.hpp>
#include <Server.hpp>
#include <utils.hpp>

//This is the top-level container for all servers.
class Container : public BaseBlock
{
  private:
    std::vector<Server> _servers;

  public:
    Container();
    ~Container();
    void insertServer(const Server &server);
};

#endif