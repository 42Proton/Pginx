#ifndef LIMITEXCEPT_HPP
#define LIMITEXCEPT_HPP

#include <utils.hpp>
#include <AccessPermission.hpp>

class LimitExcept
{
  private:
    std::set<std::string> _allowedMethods;
    AccessPermission _premissions;

  public:
    // Constructors
    LimitExcept();
    LimitExcept(const LimitExcept &copy);

    // Destructor
    ~LimitExcept();

    // Operators
    LimitExcept &operator=(const LimitExcept &assign);

    // Setters
    void setAllowedMethods(const std::string &methods);

    // Getters
    const AccessPermission& getPremissions() const;
    const std::set<std::string> &getAllowedMethods() const;

    // Memeber funtions
    bool isMethodAccepted(const std::string method) const;
};

#endif