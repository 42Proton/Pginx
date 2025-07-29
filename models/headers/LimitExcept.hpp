#ifndef LIMITEXCEPT_HPP
#define LIMITEXCEPT_HPP

#include <utils.hpp>
#include <AccessPermission.hpp>

class LimitExcept : public AccessPermission
{
  private:
    std::set<std::string> _allowedMethods;

  public:
    // Constructors
    LimitExcept();
    LimitExcept(const LimitExcept &copy);

    // Destructor
    ~LimitExcept();

    // Setters
    void setAllowedMethods(const std::set<std::string> &methods);

    // Getters
    const std::set<std::string> &getAllowedMethods() const;

    // Memeber funtions
    bool isMethodAccepted(const std::string method) const;
};

#endif