#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>

#define DEF_SYMBOL "{};"

enum TokenType {
    KEYWORD,
    NUMBER,
    STRING,
    SYMBOL
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> lexer(const std::string& content);
std::string readFile(const std::string& filename);
void checks(const std::vector<Token>& tokens);
int isAllowedTokens(const std::vector<Token>& tokens);

#endif