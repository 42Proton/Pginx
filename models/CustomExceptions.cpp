#include <CustomExceptions.hpp>

const char *CustomExceptions::OpenFileException::what() const throw()
{
    return "Open file failed";
}

const char *CustomExceptions::InititalaizingException::what() const throw()
{
    return "Error while initializing/Checking config file";
}