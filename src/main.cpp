#include <utils.hpp>

std::string checkInput(int argc, char **argv)
{
    if (argc == 1)
        return "/config/default.conf";
    if (argc == 2)
        return std::string(argv[1]);
    else
        return "";
}
int main(int argc, char **argv)
{
    std::string inputFile = checkInput(argc, argv);

    if (inputFile.empty())
    {
        std::cerr << "Invalid number of arguements" << std::endl;
        return 1;
    }

    if (parser(inputFile))
    {
        std::cerr << "Error while initializing" << std::endl;
        return 1;
    }
    std::cout << "Noicee" << std::endl;
}
