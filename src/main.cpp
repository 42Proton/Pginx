#include <defaults.hpp>
#include <utils.hpp>
#include <BaseBlock.hpp>
#include <ServerContainer.hpp>
#include <Server.hpp>

int main(int argc, char **argv)
{
    try
    {
        std::ifstream inputFile((initValidation(argc, argv)).c_str());

        if (!inputFile.is_open())
            throw CommonExceptions::OpenFileException();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    // (void)argc;
    // (void)argv;
    // BaseBlock obj;
    // obj.setRoot("");
    // std::cout << obj.getRoot() << std::endl;
    // obj.setReturnData(403);
    // std::string size = "100G";
    // obj.setClientMaxBodySize(size);
    // std::cout << obj.getClientMaxBodySize() << std::endl;
    // std::vector<std::string> routesA;
    // routesA.push_back("dir1");
    // routesA.push_back("index.html");
    // routesA.push_back("index.html/");
    // obj.insertIndex(routesA);
    // obj.getIndex();
    // std::vector<u_int16_t> errorCodes;
    // errorCodes.push_back(600);
    // errorCodes.push_back(500);
    // errorCodes.push_back(400);
    // obj.insertErrorPage(errorCodes, "error.html");
    // std::cout << obj.getErrorPage(500) << std::endl;
    return (0);
}
