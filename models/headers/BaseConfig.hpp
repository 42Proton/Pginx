#include <string>
#include <map>

//abstract holder of common directives
//reduce duplication across classes
//common stuff (root, index, error_page, etc.)

class BaseConfig {
    protected:
        std::string root;
        std::string index;
        std::map<int, std::string> errorPages;
};