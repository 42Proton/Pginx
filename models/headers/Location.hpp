#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <AccessPermission.hpp>
#include <utils.hpp>
#include <Server.hpp>

class Location : public BaseBlock, public AccessPermission
{
  public:
    // Constructors
    Location();
    Location(const Location& copy);
    Location(const Server& parent);

    // Destructor
    ~Location();
};

#endif