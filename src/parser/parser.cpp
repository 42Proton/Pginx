#include <Container.hpp>
#include <parser.hpp>
#include <utils.hpp>

bool expect(std::string expected, Token token)
{
    return token.value == expected;
}

void parser(std::vector<Token> tokens)
{

    if (tokens.empty())
        throw std::runtime_error("Empty configuration");
    if (!expect("http", tokens[0]))
        throw std::runtime_error("Expected 'http'");

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == KEYWORD)
        {
            // TODO:
        }
        if (tokens[i].type == SYMBOL)
        {
                }
    }
}

/*

http {
    server {
        listen 80;
        server_name localhost;

        location / {
            root html;
            index index.html;
        }
    }
}

*/