#include <Limit.hpp>

// Constructors
Limit::Limit()
{
	__allow = """";
	__deny = """";
	__return = {443, ""};
	std::cout << "\e[0;33mDefault Constructor called of Limit\e[0m" << std::endl;
}

Limit::Limit(const Limit &copy)
{
	__allow = copy.getAllow();
	__deny = copy.getDeny();
	__return = copy.getReturn();
}

Limit::Limit(std::string _allow, std::string _deny, std::pair<short, std::string> _return)
{
	__allow = _allow;
	__deny = _deny;
	__return = _return;
}


// Destructor
Limit::~Limit()
{
}


// Operators
Limit & Limit::operator=(const Limit &assign)
{
	__allow = assign.getAllow();
	__deny = assign.getDeny();
	__return = assign.getReturn();
	return *this;
}


// Getters / Setters
std::string Limit::getAllow() const
{
	return __allow;
}
void Limit::setAllow(std::string _allow)
{
	__allow = _allow;
}

std::string Limit::getDeny() const
{
	return __deny;
}
void Limit::setDeny(std::string _deny)
{
	__deny = _deny;
}

std::pair<short, std::string> Limit::getReturn() const
{
	return __return;
}
void Limit::setReturn(std::pair<short, std::string> _return)
{
	__return = _return;
}

// Member functions
bool Limit::methodAccepted(const std::string method) const
{
	if (_allowedMethods.find(method) != _allowedMethods.end())
		return true;
	else
		return false;
}
