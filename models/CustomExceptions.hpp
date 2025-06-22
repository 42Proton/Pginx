#ifndef CUSTOM_EXCE_HPP
#define CUSTOM_EXCE_HPP

#include <exception>
#include <utils.hpp>

class CustomExceptions
{

  public:
    class OpenFileException : public std::exception
    {
      public:
        const char *what() const throw();
    };
    class InititalaizingException : public std::exception
    {
      public:
        const char *what() const throw();
    };
};

#endif