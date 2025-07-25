#include <CommonExceptions.hpp>

const char *CommonExceptions::OpenFileException::what() const throw()
{
    return "Open file failed";
}

const char *CommonExceptions::InititalaizingException::what() const throw()
{
    return "Error while initializing/Checking config file";
}