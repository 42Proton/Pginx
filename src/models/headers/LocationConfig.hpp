#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <BaseBlock.hpp>
#include <vector>

class LocationConfig : public BaseBlock
{
  private:
    std::string _path;
    std::vector<std::string> _methods;
    std::string _uploadDir;

  public:
    LocationConfig();
    LocationConfig(const std::string &path);
    LocationConfig(const LocationConfig &obj);
    ~LocationConfig();
    
    // Setters
    void setPath(const std::string &path);
    void addMethod(const std::string &method);
    void setMethods(const std::vector<std::string> &methods);
    void setUploadDir(const std::string &dir);
    
    // Getters
    const std::string &getPath() const;
    const std::vector<std::string> &getMethods() const;
    bool isMethodAllowed(const std::string &method) const;
    const std::string &getUploadDir() const;
};

#endif