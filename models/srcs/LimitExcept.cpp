#include <LimitExcept.hpp>

// Constructors
LimitExcept::LimitExcept()
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

// Operators
LimitExcept &LimitExcept::operator=(const LimitExcept &assign)
{
    _allowedMethods = assign.getAllowedMethods();
    return *this;
}

// Setters
void LimitExcept::setAllowedMethods(const std::string &methods)
{
	std::vector<std::string> tmp = split(methods, " ");
	_allowedMethods.insert(tmp.begin(), tmp.end());
}

// Getters

const AccessPermission& LimitExcept::getPremissions() const
{
	return _premissions;
}

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
