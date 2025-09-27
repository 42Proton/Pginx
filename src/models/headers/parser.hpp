#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

#define DEF_SYMBOL "{};"

enum TokenType
{
    ATTRIBUTE,
    LEVEL,
    KEYWORD,
    NUMBER,
    STRING,
    SYMBOL
};

struct Token
{
    TokenType type;
    std::string value;
    int quoted;
};

// Forward declaration to avoid circular includes
class Container;

std::vector<Token> lexer(const std::string &content);
std::string readFile(const std::string &filename);
void checks(const std::vector<Token> &tokens);
int isAllowedTokens(const std::vector<Token> &tokens);
Container parser(const std::vector<Token> &tokens);

#endif