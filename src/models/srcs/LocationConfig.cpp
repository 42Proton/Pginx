#include <LocationConfig.hpp>

LocationConfig::LocationConfig() : BaseBlock(), _path("/"), _matchType(PREFIX)
{
    // Default allowed methods
    _methods.push_back("GET");
    _methods.push_back("POST");
    _methods.push_back("DELETE");
}

LocationConfig::LocationConfig(const std::string &path) : BaseBlock(), _path(path), _matchType(PREFIX)
{
    // Default allowed methods
    _methods.push_back("GET");
    _methods.push_back("POST");
    _methods.push_back("DELETE");
}

LocationConfig::LocationConfig(const std::string &path, MatchType matchType) : BaseBlock(), _path(path), _matchType(matchType)
{
    // Default allowed methods
    _methods.push_back("GET");
    _methods.push_back("POST");
    _methods.push_back("DELETE");
}

LocationConfig::LocationConfig(const LocationConfig &obj) : BaseBlock(obj), _path(obj._path), _matchType(obj._matchType), _methods(obj._methods), _uploadDir(obj._uploadDir), _chunked_transfer_encoding(obj._chunked_transfer_encoding)
{
}

LocationConfig::~LocationConfig()
{
}

void LocationConfig::setPath(const std::string &path)
{
    this->_path = path;
}

void LocationConfig::setMatchType(MatchType matchType)
{
    this->_matchType = matchType;
}

MatchType LocationConfig::getMatchType() const
{
    return this->_matchType;
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

void LocationConfig::setTransferEncoding(bool enabled)
{
    this->_chunked_transfer_encoding = enabled;
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