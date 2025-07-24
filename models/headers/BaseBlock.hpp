#ifndef BLOCKSERVER_HPP
#define BLOCKSERVER_HPP

#include <utils.hpp>

class BaseBlock
{
  protected:
    size_t _clientMaxBody;
    std::vector<std::string> _indexFiles;
    std::map<u_int16_t, std::string> _errorPages;
    bool _autoIndex;
    BaseBlock();
    virtual ~BaseBlock() {};

  public:
    size_t getClientMaxBody() const;
    const std::map<u_int16_t, std::string> &getErrorPages() const;
    bool getAutoIndex() const;
    void setClientMaxBody(const size_t clientMaxBody);
    void setErrorPages(const std::map<u_int16_t, std::string> &errorPages);
    void setAutoIndex(const bool autoIndex);
};

#endif