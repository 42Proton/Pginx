#include <CustomExceptions.hpp>

const char *CustomExceptions::OpenFileException::what() const throw()
{
    return "Open file failed";
}