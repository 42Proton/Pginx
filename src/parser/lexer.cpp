#include <iostream>
#include <parser.hpp>
#include <stdio.h>
#include <string>
#include <vector>

// std::string(1, *it) -> std::string(size_t n, char c)
// std::string does not have a constructor that takes a single char

bool isLevel(const std::string &s)
{
    return s == "server" || s == "http" || s == "location";
}
bool isAttribute(const std::string &s)
{
    return s == "root" || s == "client_max_body_size" || s == "listen" || s == "index" || s == "error_page" ||
           s == "server_name";
}
bool isAllDigits(const std::string &s)
{
    for (size_t i = 0; i < s.size(); ++i)
        if (!isdigit(s[i]))
            return false;
    return !s.empty();
}

static Token handleQuoted(std::string::const_iterator &it, const std::string &content)
{
    char quoteChar = *it;
    ++it;
    std::string buffer;

    while (it != content.end() && *it != quoteChar)
    {
        buffer += *it;
        ++it;
    }
    if (it == content.end())
    {
        throw std::runtime_error("Unclosed quote");
    }
    ++it;

    Token token;
    token.type = STRING;
    token.value = buffer;
    token.quoted = 1;
    return token;
}

static Token handleSymbol(std::string::const_iterator &it)
{
    Token token;
    token.type = SYMBOL;
    token.value = std::string(1, *it);
    token.quoted = 0;
    ++it;
    return token;
}

static Token handleWord(std::string::const_iterator &it, const std::string &content)
{
    std::string buffer;
    while (it != content.end() && !isspace(*it) && std::string(DEF_SYMBOL).find(*it) == std::string::npos)
    {
        buffer += *it;
        ++it;
    }

    Token token;
    if (isAllDigits(buffer))
        token.type = NUMBER;
    else if (isAttribute(buffer))
        token.type = KEYWORD;
    else
        token.type = STRING;

    token.value = buffer;
    token.quoted = 0;
    return token;
}

std::vector<Token> lexer(const std::string &content)
{
    std::vector<Token> tokens;
    std::string::const_iterator it = content.begin();

    while (it != content.end())
    {
        if (isspace(*it))
        {
            ++it;
            continue;
        }

        if (*it == '"' || *it == '\'')
        {
            tokens.push_back(handleQuoted(it, content));
        }
        else if (std::string(DEF_SYMBOL).find(*it) != std::string::npos)
        {
            tokens.push_back(handleSymbol(it));
        }
        else
        {
            tokens.push_back(handleWord(it, content));
        }
    }
    return tokens;
}

int isAllowedTokens(const std::vector<Token> &tokens)
{
    for (std::vector<Token>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        const std::string &val = it->value;

        if (it->type == SYMBOL)
        {
            if (std::string(DEF_SYMBOL).find(val[0]) == std::string::npos)
            {
                throw std::runtime_error("Invalid symbol: " + val);
            }
        }
        else if (it->type == NUMBER)
        {
            for (size_t i = 0; i < val.size(); i++)
            {
                if (!isdigit(val[i]))
                {
                    throw std::runtime_error("Invalid number: " + val);
                }
            }
        }
        else if (it->type == STRING || it->type == KEYWORD)
        {
            for (size_t i = 0; i < val.size(); i++)
            {
                char c = val[i];
                if (!isalnum(c) && c != '_' && c != '.' && c != '/' && c != '-' && c != '=' && it->quoted == 0)
                {
                    throw std::runtime_error("Invalid identifier: " + val);
                }
            }
        }
    }
    return 0;
}

void checks(const std::vector<Token> &tokens)
{

    isAllowedTokens(tokens);
    // later add: checkScopes(tokens);
    // later add: checkSemicolons(tokens);
}
