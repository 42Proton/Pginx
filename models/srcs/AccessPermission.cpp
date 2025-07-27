#include <AccessPermission.hpp>

// Constructors
AccessPermission::AccessPermission()
{
    _allow = "";
    _deny = "";
}

AccessPermission::AccessPermission(const AccessPermission &copy)
{
    _allow = copy.getAllow();
    _deny = copy.getDeny();
}

// Destructor
AccessPermission::~AccessPermission()
{
}

// Operators
AccessPermission &AccessPermission::operator=(const AccessPermission &assign)
{
    _allow = assign.getAllow();
    _deny = assign.getDeny();
    return *this;
}

// Setters

void AccessPermission::setDeny(std::string& deny)
{
    _deny = deny;
}

void AccessPermission::setAllow(std::string& allow)
{
    _allow = allow;
}

// Getters
const std::string& AccessPermission::getAllow() const
{
    return _allow;
}

const std::string& AccessPermission::getDeny() const
{
    return _deny;
}


bool  AccessPermission::isIpAccepted(const std::string& Ip) const
{
    if (Ip == _allow)
        return true;
    else if (Ip == _deny || _deny == "any")
        return false;
    else
        return true;
}