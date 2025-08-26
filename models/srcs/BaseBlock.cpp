#include <BaseBlock.hpp>

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

void BaseBlock::setReturnData(const u_int16_t code, const std::string &route)
{
    if (code > 999)
        throw CommonExceptions::InvalidStatusCode();
    this->_returnData.first = code;
    this->_returnData.second = route;
}

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

void BaseBlock::insertIndex(const std::vector<std::string> &routes)
{
    size_t len = routes.size();
    for (size_t i = 0; i < len; i++)
        this->_indexFiles.push_back(routes[i]);
}

void BaseBlock::insertErrorPage(const std::vector<u_int16_t> &errorCodes, const std::string &errorPage)
{
    this->_errorPagesCache.insert(errorPage);
    const std::string &pageRef = *this->_errorPagesCache.find(errorPage);
    size_t len = errorCodes.size();
    for (size_t i = 0; i < len; i++)
    {
        if (errorCodes[i] < 300 || errorCodes[i] > 599)
            throw CommonExceptions::InvalidValue();
        this->_errorPages[errorCodes[i]] = &pageRef;
    }
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

const std::string BaseBlock::getIndex() const
{
    struct stat statBuf;
    size_t len = this->_indexFiles.size();
    std::string currentRoot = this->_root;

    for (size_t i = 0; i < len; i++)
    {
        std::string index_path = currentRoot + this->_indexFiles[i];
        if (access(index_path.c_str(), F_OK))
            continue;
        if (stat(index_path.c_str(), &statBuf) == -1)
            throw CommonExceptions::StatError();
        if (S_ISDIR(statBuf.st_mode))
        {
            currentRoot.append(this->_indexFiles[i]);
            if (str_back(currentRoot) != '/')
                currentRoot.push_back('/');
        }
        else if (S_ISREG(statBuf.st_mode))
        {
            if (access(index_path.c_str(), R_OK))
                throw CommonExceptions::ForbiddenAccess();
            return index_path;
        }
        else
            throw CommonExceptions::NotRegularFile();
    }
    throw CommonExceptions::NoAvailablePage();
}

const std::string BaseBlock::getErrorPage(const u_int16_t code) const
{
    std::map<u_int16_t, const std::string *>::const_iterator cIt = this->_errorPages.find(code);
    if (cIt == this->_errorPages.end())
        throw CommonExceptions::NoAvailablePage();

    struct stat statBuf;
    std::string page_path = this->_root + *(*cIt).second;
    if (access(page_path.c_str(), F_OK))
        throw CommonExceptions::NoAvailablePage();
    if (stat(page_path.c_str(), &statBuf) == -1)
        throw CommonExceptions::StatError();
    if (!S_ISREG(statBuf.st_mode))
        throw CommonExceptions::NoAvailablePage();
    if (access(page_path.c_str(), R_OK))
        throw CommonExceptions::ForbiddenAccess();
    return page_path;
}

bool BaseBlock::getAutoIndex() const
{
    return this->_autoIndex;
}