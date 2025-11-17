#include <BaseBlock.hpp>
#include <cerrno>

BaseBlock::BaseBlock()
    : _root(DEFAULT_ROOT_PATH), _returnData(404, ""), _clientMaxBodySize(1048576), _indexFiles(), _errorPages(),
      _autoIndex(false)
{
}

BaseBlock::BaseBlock(const BaseBlock &obj)
    : _root(obj._root), _returnData(obj._returnData), _clientMaxBodySize(obj._clientMaxBodySize),
      _indexFiles(obj._indexFiles), _errorPages(obj._errorPages), _autoIndex(obj._autoIndex)
{
}

void BaseBlock::setRoot(const std::string &root)
{
    this->_root.clear();
    if (!root.size() || root[0] != '/')
        this->_root = PGINX_PREFIX;
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