#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>
#include <AccessPermission.hpp>

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
    bool operator==(const ListenCtx &other) const
    {
        return this->port == other.port && this->addr == other.addr;
    }
    bool operator!=(const ListenCtx &other) const
    {
        return !(*this == other);
    }
};

class Server : public BaseBlock, public AccessPermission
{
  private:
    std::vector<ListenCtx> _listens;
    std::vector<std::string> _serverNames;
    std::string _root;

    // Location Variable is yet to be defiend until Amjad implements it.
    bool validateAddress(const std::string &addr) const;

  public:
    Server();
    ~Server() {};
    const std::vector<ListenCtx> &getListens() const;
    const std::vector<std::string> &getServerNames() const;
    void insertListen(u_int16_t port = 80, const std::string &addr = "0.0.0.0");
    void insertServerNames(const std::string &serverName);
    void setRoot(const std::string &root = "pages/");
    const std::string &getRoot() const;
    void setIndexFiles(const std::vector<std::string> &indexFiles);
    const std::vector<std::string> &getIndexFiles() const;
};
#endif