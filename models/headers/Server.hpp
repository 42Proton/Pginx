#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>

// Listen Context
/**
 *
 *Why not pairs? Simply because we can add to the struct without changing anything in the already existing code.
 * And naming Convention is used to make it clear that this struct is used for listening purposes.
 */
struct ListenCtx
{
    u_int16_t port;
    std::string addr;
};

class Server : public BaseBlock
{
  private:
    ListenCtx _listen;
    std::vector<std::string> _serverNames;
    std::string _root;
    std::vector<std::string> _indexFiles;
    std::map<u_int16_t, std::string> _errorPages;
    size_t _clientMaxBody;

    // Location Variable is yet to be defiend until Amjad implements it.
    bool validatePort(u_int16_t port) const;
    bool validateAddress(const std::string &addr) const;

  public:
    Server();
    const ListenCtx &getListen() const;
    const std::vector<std::string> &getServerNames() const;
    void setListen(u_int16_t port, const std::string &addr);
    void insertServerNames(const std::string &serverName);
    void setRoot(const std::string &root);
    const std::string &getRoot() const;
    void setIndexFiles(const std::vector<std::string> &indexFiles);
    const std::vector<std::string> &getIndexFiles() const;
    void setErrorPages(const std::map<u_int16_t, std::string> &errorPages);
    const std::map<u_int16_t, std::string> &getErrorPages() const;
    void setClientMaxBody(const size_t clientMaxBody);
    size_t getClientMaxBody() const;
};

#endif