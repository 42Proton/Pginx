#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <Server.hpp>

class Container : public BaseBlock {
  private:
    std::vector<Server> _servers;

  public:
    Container();
    ~Container();
    void insertServer(const Server &server);
    const std::vector<Server> &getServers() const;
};

#endif