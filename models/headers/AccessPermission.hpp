#ifndef ACCESSPERMISSION_HPP
#define ACCESSPERMISSION_HPP

#include <utils.hpp>

class AccessPermission
{
  protected:
    std::set<std::string> _allow;
    std::set<std::string> _deny;

  public:
    // Constructors
    AccessPermission();
    AccessPermission(const AccessPermission &copy);

    // Destructor
    ~AccessPermission();

    // Operators
    AccessPermission &operator=(const AccessPermission &assign);

    // Setters
    void insertAllow(std::string &allow);
    void insertDeny(std::string &deny);

    // Getters
    const std::set<std::string> &getAllow() const;
    const std::set<std::string> &getDeny() const;

    // Memeber functions
    bool  isIpAccepted(const std::string& Ip) const;
};

#endif