#include <LocationConfig.hpp>

LocationConfig::LocationConfig() : BaseBlock(), _path("/")
{
    // Default allowed methods
    _methods.push_back("GET");
    _methods.push_back("POST");
    _methods.push_back("DELETE");
}

LocationConfig::LocationConfig(const std::string &path) : BaseBlock(), _path(path)
{
    // Default allowed methods
    _methods.push_back("GET");
    _methods.push_back("POST");
    _methods.push_back("DELETE");
}

LocationConfig::LocationConfig(const LocationConfig &obj) : BaseBlock(obj), _path(obj._path), _methods(obj._methods)
{
}

LocationConfig::~LocationConfig()
{
}

void LocationConfig::setPath(const std::string &path)
{
    this->_path = path;
}

void LocationConfig::setUploadDir(const std::string &dir) {
    _uploadDir = dir; 
}

const std::string& LocationConfig::getUploadDir() const { 
    return _uploadDir;
}

void LocationConfig::addMethod(const std::string &method)
{
    // Check if method already exists to avoid duplicates
    for (std::vector<std::string>::const_iterator it = _methods.begin(); it != _methods.end(); ++it)
    {
        if (*it == method)
            return;
    }
    this->_methods.push_back(method);
}

void LocationConfig::setMethods(const std::vector<std::string> &methods)
{
    this->_methods = methods;
}

const std::string &LocationConfig::getPath() const
{
    return this->_path;
}

const std::vector<std::string> &LocationConfig::getMethods() const
{
    return this->_methods;
}

//This function checks if a specific HTTP method (like "GET", "POST", "DELETE") is allowed for this location.
bool LocationConfig::isMethodAllowed(const std::string &method) const
{
    for (std::vector<std::string>::const_iterator it = _methods.begin(); it != _methods.end(); ++it)
    {
        if (*it == method)
            return true;
    }
    return false;
}