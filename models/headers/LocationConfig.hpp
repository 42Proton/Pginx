#include <BaseBlock.hpp>
#include <vector>

class LocationConfig : public BaseBlock
{
  private:
    std::string path;
    std::vector<std::string> methods;
};