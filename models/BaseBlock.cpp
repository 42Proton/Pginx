#include <BaseBlock.hpp>

BaseBlock::BaseBlock():
	_clientMaxBody(0),
	_autoIndex(false)
{
	_errorPages[404] = "pages/404.html";
    _errorPages[405] = "pages/405.html";
    _errorPages[500] = "pages/500.html";
}

size_t  BaseBlock::getClientMaxBody() const
{
	return this->_clientMaxBody;
}

const std::map<u_int16_t, std::string>&  BaseBlock::getErrorPages() const
{
	return this->_errorPages;
}

bool  BaseBlock::getAutoIndex() const
{
	return this->_autoIndex;
}

void    BaseBlock::setClientMaxBody(const size_t clientMaxBody)
{
	this->_clientMaxBody = clientMaxBody;
}

void    BaseBlock::setErrorPages(const std::map<u_int16_t, std::string>& errorPages)
{
	this->_errorPages = errorPages;
}

void    BaseBlock::setAutoIndex(const bool autoIndex)
{
	this->_autoIndex = autoIndex;
}