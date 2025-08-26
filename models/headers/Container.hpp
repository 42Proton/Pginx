#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <BaseBlock.hpp>
#include <Server.hpp>
#include <utils.hpp>

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