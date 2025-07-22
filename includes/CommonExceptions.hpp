#ifndef CUSTOM_EXCE_HPP
#define CUSTOM_EXCE_HPP

#include <exception>
#include <utils.hpp>

class CommonExceptions
{
  private:
    CommonExceptions();
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