#include <BaseBlock.hpp>
#include <cerrno>

BaseBlock::BaseBlock()
    : _root(DEFAULT_ROOT_PATH), _returnData(404, ""), _clientMaxBodySize(1048576), _indexFiles(), _errorPages(),
      _autoIndex(false), _cgiEnabled(false), _cgiExplicitlySet(false), _cgiPassMap()
{
}

BaseBlock::BaseBlock(const BaseBlock &obj)
    : _root(obj._root), _returnData(obj._returnData), _clientMaxBodySize(obj._clientMaxBodySize),
      _indexFiles(obj._indexFiles), _errorPages(obj._errorPages), _autoIndex(obj._autoIndex), 
      _cgiEnabled(obj._cgiEnabled), _cgiExplicitlySet(obj._cgiExplicitlySet), _cgiPassMap(obj._cgiPassMap)
{
}

void BaseBlock::setRoot(const std::string &root)
{
    this->_root.clear();
    if (!root.size() || root[0] != '/')
    {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            this->_root = std::string(cwd) + "/";
        else
            this->_root = PGINX_PREFIX;
    }
    this->_root.append(root);
    if (str_back(root) != '/')
        this->_root.push_back('/');
}
BaseBlock::~BaseBlock() {};

void BaseBlock::setClientMaxBodySize(std::string &sSize)
{
    char sizeCategory = 0;
    char *endptr;

    if (sSize.empty() || sSize.find('.') != std::string::npos)
        throw CommonExceptions::InvalidValue();
    if (!isdigit(str_back(sSize)))
    {
        sizeCategory = tolower(str_back(sSize));
        sSize.erase(sSize.size() - 1);
    }
    this->_clientMaxBodySize = strtoul(sSize.c_str(), &endptr, 10);
    if (*endptr || errno == ERANGE)
        throw CommonExceptions::InvalidValue();
    switch (sizeCategory)
    {
    case 0:
        return;
    case 'k':
        if (this->_clientMaxBodySize > MAX_KILOBYTE)
            throw CommonExceptions::InvalidValue();
        this->_clientMaxBodySize *= KILOBYTE;
        return;
    case 'm':
        if (this->_clientMaxBodySize > MAX_MEGABYTE)
            throw CommonExceptions::InvalidValue();
        this->_clientMaxBodySize *= MEGABYTE;
        return;
    case 'g':
        if (this->_clientMaxBodySize > MAX_GIGABYTE)
            throw CommonExceptions::InvalidValue();
        this->_clientMaxBodySize *= GIGABYTE;
        return;
    default:
        throw CommonExceptions::InvalidValue();
    }
}

void BaseBlock::setCgiEnabled(bool enabled) {
  this->_cgiEnabled = enabled;
  this->_cgiExplicitlySet = true;
}

bool BaseBlock::isCgiEnabled() const {
  return this->_cgiEnabled;
}

bool BaseBlock::isCgiExplicitlySet() const {
  return this->_cgiExplicitlySet;
}

void BaseBlock::inheritCgiFromParent(bool parentCgiEnabled) {
  if (!this->_cgiExplicitlySet) {
    this->_cgiEnabled = parentCgiEnabled;
    // Don't set _cgiExplicitlySet to true - this is inheritance, not explicit
  }
}

void BaseBlock::setCgiPassMapping(const std::string &extension, const std::string &interpreterPath) {
    this->_cgiPassMap[extension] = interpreterPath;
}

std::map<std::string, std::string> BaseBlock::getCgiPassMap() const {
    return this->_cgiPassMap;
}

void BaseBlock::inheritCgiPassFromParent(const std::map<std::string, std::string> &parentCgiPassMap) {
    // Inherit parent mappings, but don't override existing location-specific mappings
    for (std::map<std::string, std::string>::const_iterator it = parentCgiPassMap.begin(); 
         it != parentCgiPassMap.end(); ++it) {
        if (this->_cgiPassMap.find(it->first) == this->_cgiPassMap.end()) {
            // Extension not defined in location, inherit from parent
            this->_cgiPassMap[it->first] = it->second;
        }
        // If extension already exists in location, keep location's value (override)
    }
}


void BaseBlock::insertIndex(const std::vector<std::string> &indexFiles) {
    _indexFiles.clear();
    for (size_t i = 0; i < indexFiles.size(); ++i) {
        if (!indexFiles[i].empty())
            _indexFiles.push_back(indexFiles[i]);
    }

    if (_indexFiles.empty())
        _indexFiles.push_back("index.html");
}

static bool isHttpErrorCode(u_int16_t code) {
    return code >= 300 && code <= 599;
}

void BaseBlock::insertErrorPage(u_int16_t errorCode, const std::string &errorPage) {
    if (!isHttpErrorCode(errorCode))
        throw CommonExceptions::InvalidValue();
    _errorPages[errorCode] = errorPage;
}

const std::string *BaseBlock::getErrorPage(const u_int16_t code) const
{
    std::map<u_int16_t, std::string>::const_iterator cIt = this->_errorPages.find(code);
    if (cIt == this->_errorPages.end())
        return NULL;
    return &cIt->second;
}

void BaseBlock::activateAutoIndex()
{
    this->_autoIndex = true;
}

const std::string &BaseBlock::getRoot() const
{
    return this->_root;
}

const std::pair<u_int16_t, std::string> &BaseBlock::getReturnData() const
{
    return this->_returnData;
}

size_t BaseBlock::getClientMaxBodySize() const
{
    return this->_clientMaxBodySize;
}

const std::vector<std::string> &BaseBlock::getIndexFiles() const
{
    return this->_indexFiles;
}


bool BaseBlock::getAutoIndex() const
{
    return this->_autoIndex;
}