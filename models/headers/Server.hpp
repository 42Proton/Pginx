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
    std::string interface;
};

class Server : public BaseBlock
{
  private:
    std::vector<ListenCtx> _listens;
    std::string _serverName;

  public:
    Server();
    const std::vector<ListenCtx> &getListens() const;
    const std::string &getServerName() const;
    void setListens(const std::vector<ListenCtx> &listens);
    void setServerName(const std::string &serverName);
};

#endif