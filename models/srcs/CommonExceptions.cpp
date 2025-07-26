#include <CommonExceptions.hpp>

const char *CommonExceptions::OpenFileException::what() const throw()
{
    return "Open file failed";
}

const char *CommonExceptions::InititalaizingException::what() const throw()
{
    return "Error while initializing/Checking config file";
}

const char *CommonExceptions::InvalidValue::what() const throw()
{
    return "directive invalid value";
}

const char *CommonExceptions::NotRegularFile::what() const throw()
{
    return "file is not regular type";
}

const char *CommonExceptions::NoAvailablePage::what() const throw()
{
    return "no available page";
}

const char *CommonExceptions::ForbiddenAccess::what() const throw()
{
    return "403 Forbidden";
}

const char *CommonExceptions::StatError::what() const throw()
{
    return "stat error";
}

const char *CommonExceptions::InvalidStatusCode::what() const throw()
{
    return "invalid status code";
}