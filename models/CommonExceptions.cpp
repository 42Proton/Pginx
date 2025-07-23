#include <CommonExceptions.hpp>

const char *CommonExceptions::OpenFileException::what() const throw()
{
    return "Open file failed";
}