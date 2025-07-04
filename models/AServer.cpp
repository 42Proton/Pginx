#include <AServer.hpp>

AServer::AServer() : _clientMaxBody(0), _autoIndex(false)
{
    _errorPages[404] = "pages/404.html";
    _errorPages[405] = "pages/405.html";
    _errorPages[500] = "pages/500.html";
}
size_t AServer::getClientMaxBody() const
{
    return _clientMaxBody;
}

std::map<u_int16_t, std::string> AServer::getErrorPages() const
{
    return _errorPages;
}

bool AServer::getAutoIndex() const
{
    return _autoIndex;
}
void AServer::setClientMaxBody(size_t clientMaxBody)
{
    _clientMaxBody = clientMaxBody;
}

void AServer::setErrorPages(std::map<u_int16_t, std::string> errorPages)
{
    _errorPages = errorPages;
}

void AServer::setAutoIndex(bool autoIndex)
{
    _autoIndex = autoIndex;
}