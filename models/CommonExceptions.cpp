#include <CommonExceptions.hpp>

const char *CommonExceptions::OpenFileException::what() const throw()
{
    return "CommonExceptions::OpenFileException";
}

const char *CommonExceptions::InititalaizingException::what() const throw()
{
    return "Error while initializing/Checking config file";
}