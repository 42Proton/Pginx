#ifndef COMMON_EXCEPTIONS_HPP
#define COMMON_EXCEPTIONS_HPP

#include <exception>
#include <utils.hpp>

class CommonExceptions {
  private:
    CommonExceptions();

  public:
    class OpenFileException : public std::exception {
      public:
        const char *what() const throw();
    };

    class InititalaizingException : public std::exception {
      public:
        const char *what() const throw();
    };
    
    class InvalidValue : public std::exception {
      public:
        const char *what() const throw();
    };

    class NotRegularFile : public std::exception {
      public:
        const char *what() const throw();
    };

    class NoAvailablePage : public std::exception {
      public:
        const char *what() const throw();
    };

    class ForbiddenAccess : public std::exception {
      public:
        const char *what() const throw();
    };

    class StatError : public std::exception {
      public:
        const char *what() const throw();
    };

    class InvalidStatusCode : public std::exception {
      public:
        const char *what() const throw();
    };
};

#endif