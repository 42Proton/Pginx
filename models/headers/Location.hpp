#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <AccessPermission.hpp>
#include <utils.hpp>
#include <BaseBlock.hpp>

class Location : public BaseBlock
{
  private:
    AccessPermission _permission;

  public:
    // Constructors
    Location();
    Location(const Location &copy);
    Location(AccessPermission permission);

    // Destructor
    ~Location();

    // Operators
    Location &operator=(const Location &assign);

    // Getters / Setters
    AccessPermission getPermission() const;
};

#endif