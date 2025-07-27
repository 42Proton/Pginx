#ifndef ACCESSPERMISSION_HPP
#define ACCESSPERMISSION_HPP

#include <utils.hpp>

class AccessPermission
{
  private:
    std::string _allow;
    std::string _deny;

  public:
    // Constructors
    AccessPermission();
    AccessPermission(const AccessPermission &copy);

    // Destructor
    ~AccessPermission();

    // Operators
    AccessPermission &operator=(const AccessPermission &assign);

    // Setters
    void setAllow(std::string &allow);
    void setDeny(std::string &deny);

    // Getters
    const std::string &getAllow() const;
    const std::string &getDeny() const;

    // Memeber functions
    bool  isIpAccepted(const std::string& Ip) const;
};

#endif