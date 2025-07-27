#include "Location.hpp"

// Constructors
Location::Location()
{
}

Location::Location(const Location &copy)
{
    _permission = copy.getPermission();
}

Location::Location(AccessPermission permission)
{
    _permission = permission;
}

// Destructor
Location::~Location()
{
}

// Operators
Location &Location::operator=(const Location &assign)
{
    _permission = assign.getPermission();
    return *this;
}

// Getters / Setters
AccessPermission Location::getPermission() const
{
    return _permission;
}
