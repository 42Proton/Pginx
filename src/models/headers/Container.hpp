#ifndef SERVERCONTAINER_HPP
#define SERVERCONTAINER_HPP

#include <BaseBlock.hpp>
#include <Server.hpp>
#include <utils.hpp>

// This is the top-level container for all servers.
class Container : public BaseBlock {
  private:
    std::vector<Server> _servers;
    std::set<u_int16_t> _ports;
    std::map<std::string, u_int16_t> addrPortMap;

  public:
    Container();
    ~Container();
    void insertServer(const Server &server);
    const std::vector<Server> &getServers() const;
};

#endif