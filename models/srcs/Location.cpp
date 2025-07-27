#include "Location.hpp"

// Constructors
Location::Location() : BaseBlock(), AccessPermission()
{
}

Location::Location(const Location &copy) : BaseBlock(copy), AccessPermission(copy)
{
}

Location::Location(const Server &parent) : BaseBlock(parent), AccessPermission(parent)
{
}

// Destructor
Location::~Location()
{
}
