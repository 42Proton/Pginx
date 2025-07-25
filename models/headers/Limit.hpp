#ifndef LIMIT_HPP
# define LIMIT_HPP

# include <utility>
# include <iostream>
# include <string>
# include <set>

class Limit
{
	public:
		// Constructors
		Limit();
		Limit(const Limit &copy);
		Limit(std::string allow, std::string deny, std::pair<short, std::string> return);
		
		// Destructor
		~Limit();
		
		// Operators
		Limit & operator=(const Limit &assign);
		
		// Getters / Setters
		std::string getAllow() const;
		void setAllow(std::string _allow);
		std::string getDeny() const;
		void setDeny(std::string _deny);
		std::pair<short, std::string> getReturn() const;
		void setReturn(std::pair<short, std::string> _return);

		// Memeber funtions
		bool methodAccepted(const std::string method) const;
		
	private:
		std::set<std::string> _allowedMethods;
		std::string __allow;
		std::string __deny;
		std::pair<short, std::string> __return;
		
};

#endif