#include <defaults.hpp>
#include <sstream>
#include <utils.hpp>

static std::string checkInput(int argc, char **argv)
{
    if (argc == 1)
        return DEFAULT_PATH;
    if (argc == 2)
        return std::string(argv[1]);
    else
        return "";
}

static bool checkValidExt(std::string input)
{
    if (input.empty() || input.length() < 5 || input.length() > MAX_EXT_LENGTH)
        return true;
    size_t dotIndex = input.find_last_of('.');

    if (dotIndex == std::string::npos || dotIndex == 0 || dotIndex == input.length() - 1)
        return true;
    if (input.find('.') != dotIndex)
        return true;

    std::string extension = input.substr(dotIndex + 1);
    if (extension != "conf")
        return true;
    return false;
}

std::string initValidation(int argc, char **argv)
{

    std::string inputFile = checkInput(argc, argv);

    if (inputFile.empty() || checkValidExt(inputFile))
        throw CommonExceptions::InititalaizingException();
    return (inputFile);
}
