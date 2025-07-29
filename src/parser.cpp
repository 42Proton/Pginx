#include <fstream>
#include <sstream>
#include <utils.hpp>

static bool checkValidExt(std::string input)
{
    if (input.empty())
        return true;
    std::stringstream ss(input);
    std::string temp;
    int count = 0;
    while (std::getline(ss, temp, '.'))
    {
        if (count > 2)
            return true;
        count++;
    }

    if (count != 2)
        return true;
    if (temp.compare("conf"))
        return true;

    return false;
}

bool parser(std::string inputFile)
{
    if (checkValidExt(inputFile))
        return true;

    std::ifstream file(inputFile.c_str());
    if (!file.is_open())
        throw CommonExceptions::OpenFileException();

    return false;
}
