#include <iostream>
#include <parser.hpp>
#include <utils.hpp>
int main(int argc, char **argv)
{
    // TODO: handling default values
    if (argc == 1)
        return 0;
    if (argc != 2)
    {
        std::cerr << "Provide a Configuration file! " << std::endl;
        return 1;
    }
    try
    {
        initValidation(argc, argv);
        std::string content = readFile(argv[1]);
        std::vector<Token> tokens = lexer(content);
        checks(tokens);

        // std::cout << content << std::endl;

        // Print tokens
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            std::cout << "Type: " << tokens[i].type << " Value: " << tokens[i].value << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
