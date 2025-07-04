#ifndef ASERVER_HPP
#define ASERVER_HPP

#include <utils.hpp>

class AServer
{
  private:
    size_t _clientMaxBody;
    std::map<u_int16_t, std::string> _errorPages;
    bool _autoIndex;

  public:
    AServer();
    size_t getClientMaxBody() const;
    std::map<u_int16_t, std::string> getErrorPages() const;
    bool getAutoIndex() const;
    void setClientMaxBody(size_t clientMaxBody);
    void setErrorPages(std::map<u_int16_t, std::string> errorPages);
    void setAutoIndex(bool autoIndex);
    virtual ~AServer() {};
};

#endif