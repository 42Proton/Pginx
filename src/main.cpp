#include <defaults.hpp>
#include <utils.hpp>

int main(int argc, char **argv)
{
    try
    {
        std::ifstream inputFile((initValidation(argc, argv)).c_str());

        if (!inputFile.is_open())
            throw CustomExceptions::OpenFileException();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
