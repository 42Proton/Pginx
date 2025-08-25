#include "BaseConfig.hpp"
#include "ServerConfig.hpp"
#include <vector>

//the whole config file (list of servers, maybe globals like http { ... }

//a list of ServerConfig objects (each describing a server block)
//It is the entry point your program will use to read config.
class Config : public BaseConfig {
    private:
        std::vector<ServerConfig> servers;
};