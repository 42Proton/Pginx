#include <BaseBlock.hpp>
#include <vector>

//Represents one location { ... } block inside a server.
class LocationConfig : public BaseBlock
{
  private:
    std::string path;
    std::vector<std::string> methods;
};