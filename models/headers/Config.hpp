#include "BaseConfig.hpp"
#include "ServerConfig.hpp"
#include <vector>

//contain global directives(root, index, etc)
//a list of ServerConfig objects (each describing a server block)
//It is the entry point your program will use to read config.
class Config : public BaseConfig {
    private:
        std::vector<ServerConfig> servers;
};