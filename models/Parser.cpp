#include <Parser.hpp>

Parser::Parser(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		throw CommonExceptions::OpenFileException();
	try
	{
		file.exceptions(std::ios::badbit);
		
	}
	catch (...)
	{
		file.close();
		throw;
	}
	file.close();
}

Parser::~Parser() {}

const std::vector<Server> &Parser::getServers() const
{
    return _servers;
}

void Parser::validateServers() const
{

}
