#include <LimitExcept.hpp>

// Constructors
LimitExcept::LimitExcept() : AccessPermission()
{
}

LimitExcept::LimitExcept(const LimitExcept &copy)
{
    *this = copy;
}

// Destructor
LimitExcept::~LimitExcept()
{
}

// Setters
void LimitExcept::setAllowedMethods(const std::set<std::string> &methods)
{
	_allowedMethods.insert(methods.begin(), methods.end());
}

// Getters
const std::set<std::string>& LimitExcept::getAllowedMethods() const
{
	return _allowedMethods;
}

// Member functions
bool LimitExcept::isMethodAccepted(const std::string method) const
{
    if (_allowedMethods.find(method) != _allowedMethods.end())
        return true;
    else
        return false;
}
