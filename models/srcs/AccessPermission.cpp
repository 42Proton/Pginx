#include <AccessPermission.hpp>

// Constructors
AccessPermission::AccessPermission()
{
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

void AccessPermission::insertDeny(std::string &deny)
{
    _deny.insert(deny);
}

void AccessPermission::insertAllow(std::string &allow)
{
    _allow.insert(allow);
}

// Getters
const std::set<std::string> &AccessPermission::getAllow() const
{
    return _allow;
}

const std::set<std::string> &AccessPermission::getDeny() const
{
    return _deny;
}

bool AccessPermission::isIpAccepted(const std::string &Ip) const
{
    (void)Ip;
    return true;
}