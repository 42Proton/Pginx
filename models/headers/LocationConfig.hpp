#include "BaseConfig.hpp"
#include <vector>

class LocationConfig : public BaseConfig {
    private:
        std::string path;
        std::vector<std::string> methods;

};