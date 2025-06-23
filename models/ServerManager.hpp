#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

struct RouteConfig
{
  std::string path;
  unsigned accepted_methods; // We can check if (%2 == 0) then GET ad if (%3 == 0) then POST or use bitwise operations
  bool dir_listing;
  std::string index_file;
  std::map<std::string, std::string> cgi_handlers;
  bool upload_enabled;
  std::string upload_path;

  RouteConfig() : accepted_methods(1), dir_listing(false), upload_enabled(false)
  {
  }
};

struct ServerConfig
{
  std::vector<std::string> server_names;
  std::string root;
  std::vector<RouteConfig> routes;
};

class ServerManager
{
  protected:
  public:
    // Constructors
    ServerManager();
    ServerManager(const ServerManager &copy);

    // Destructor
    ~ServerManager();

    // Operators
    ServerManager &operator=(const ServerManager &assign);

  private:
    std::map<std::pair<std::string, int>, ServerConfig> _server;
};

#endif