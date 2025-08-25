#include <iostream>
#include "parser.hpp"

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << "Provide a Configuration file! " << std::endl;
        return 1;
    }
    try {
        std::string content = readFile(argv[1]);
        std::vector<Token> tokens = lexer(content);
        checks(tokens);

        // std::cout << content << std::endl;

        // Print tokens
        for (size_t i = 0; i < tokens.size(); ++i) {
            std::cout << "Type: " << tokens[i].type << " Value: " << tokens[i].value << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
