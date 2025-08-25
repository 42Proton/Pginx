#include "BaseConfig.hpp"
#include "LocationConfig.hpp"
#include <vector>

class ServerConfig : public BaseConfig {
    private:
        int Port;
        std::string serverName;
        std::vector<LocationConfig> locations;
};