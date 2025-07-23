#ifndef SERVER_HPP
#define SERVER_HPP

#include <BaseBlock.hpp>
class Server : public BaseBlock
{
  private:
    std::vector<u_int16_t> _ports;
    std::string _serverName;
    static std::string _defaultServerName;
    std::string access_log;
    std::string error_log;

  public:
    Server();
    bool isDefault() const;
    const std::vector<u_int16_t> &getPorts() const;
    const std::string &getServerName() const;
    void setIsDefault(const std::string &defaultServerName);
    void setPorts(const std::vector<u_int16_t> &ports);
    void setServerName(const std::string &serverName);
    const std::string &getAccessLogPath() const;
    const std::string &getErrorLogPath() const;
    void writeIntoAccessLog(const std::string &log) const;
    void writeIntoErrorLog(const std::string &log) const;
    void setAccessLogPath(const std::string &path);
    void setErrorLogPath(const std::string &path);
    static void setDefaultServerName(const std::string &defaultServerName);
    static const std::string &getDefaultServerName();
};

#endif