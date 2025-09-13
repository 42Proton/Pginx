#include <Container.hpp>
#include <cstdlib>
#include <parser.hpp>
#include <utils.hpp>

bool expect(std::string expected, Token token)
{
    return token.value == expected;
}

Container parser(const std::vector<Token> &tokens)
{
    Container container;

    if (tokens.empty())
        throw std::runtime_error("Empty configuration");
    if (!expect("http", tokens[0]))
        throw std::runtime_error("Expected 'http'");

    size_t i = 1;
    int httpBraceLevel = 0;

    // Skip opening brace of http block and count it
    if (i < tokens.size() && tokens[i].value == "{")
    {
        httpBraceLevel++;
        i++;
    }

    while (i < tokens.size() && httpBraceLevel > 0)
    {
        // Track brace levels
        if (tokens[i].value == "{")
        {
            httpBraceLevel++;
        }
        else if (tokens[i].value == "}")
        {
            httpBraceLevel--;
            if (httpBraceLevel == 0)
            {
                break;
            }
        }

        if (tokens[i].type == LEVEL && tokens[i].value == "server")
        {
            Server server;
            i++; // move past "server"

            int serverBraceLevel = 0;
            // Skip opening brace and count it
            if (i < tokens.size() && tokens[i].value == "{")
            {
                serverBraceLevel++;
                httpBraceLevel++; // Also count for http level
                i++;
            }

            // Parse server block
            while (i < tokens.size() && serverBraceLevel > 0)
            {
                // Track server brace levels
                if (tokens[i].value == "{")
                {
                    serverBraceLevel++;
                    httpBraceLevel++;
                }
                else if (tokens[i].value == "}")
                {
                    serverBraceLevel--;
                    httpBraceLevel--;
                    if (serverBraceLevel == 0)
                    {
                        i++; // Move past the closing brace
                        break;
                    }
                }

                if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL)
                {
                    std::string directive = tokens[i].value;
                    i++;

                    if (directive == "listen" && i < tokens.size())
                    {
                        // Parse port number from string
                        u_int16_t port = 80; // default
                        if (!tokens[i].value.empty())
                        {
                            port = static_cast<u_int16_t>(std::atoi(tokens[i].value.c_str()));
                        }
                        server.insertListen(port);
                        i++;
                    }
                    else if (directive == "server_name" && i < tokens.size())
                    {
                        server.insertServerNames(tokens[i].value);
                        i++;
                    }
                    else if (directive == "root" && i < tokens.size())
                    {
                        server.setRoot(tokens[i].value);
                        i++;
                    }
                    else if (directive == "index" && i < tokens.size())
                    {
                        // Handle multiple index files
                        std::vector<std::string> indexFiles;
                        while (i < tokens.size() && tokens[i].value != ";")
                        {
                            indexFiles.push_back(tokens[i].value);
                            i++;
                        }
                        server.insertIndex(indexFiles);
                    }
                    else if (directive == "client_max_body_size" && i < tokens.size())
                    {
                        std::string sizeStr = tokens[i].value;
                        server.setClientMaxBodySize(sizeStr);
                        i++;
                    }
                    else if (directive == "autoindex" && i < tokens.size())
                    {
                        if (tokens[i].value == "on")
                        {
                            server.activateAutoIndex();
                        }
                        i++;
                    }
                    else if (directive == "error_page" && i < tokens.size())
                    {
                        // Handle error_page directive: error_page 404 /page.html; or error_page 500 502 503 /page.html;
                        std::vector<u_int16_t> errorCodes;
                        std::string errorPage;

                        // Collect error codes
                        while (i < tokens.size() && tokens[i].value != ";" && tokens[i].type == NUMBER)
                        {
                            errorCodes.push_back(static_cast<u_int16_t>(std::atoi(tokens[i].value.c_str())));
                            i++;
                        }

                        // Get the error page path
                        if (i < tokens.size() && tokens[i].value != ";")
                        {
                            errorPage = tokens[i].value;
                            i++;
                        }

                        if (!errorCodes.empty() && !errorPage.empty())
                        {
                            server.insertErrorPage(errorCodes, errorPage);
                        }
                    }
                    else if (directive == "location")
                    {
                        // Parse location block
                        if (i < tokens.size())
                        {
                            std::string path = tokens[i].value;
                            i++;

                            LocationConfig location(path);

                            int locationBraceLevel = 0;
                            // Skip opening brace
                            if (i < tokens.size() && tokens[i].value == "{")
                            {
                                locationBraceLevel++;
                                serverBraceLevel++;
                                httpBraceLevel++;
                                i++;
                            }

                            // Parse location directives
                            while (i < tokens.size() && locationBraceLevel > 0)
                            {
                                if (tokens[i].value == "{")
                                {
                                    locationBraceLevel++;
                                    serverBraceLevel++;
                                    httpBraceLevel++;
                                }
                                else if (tokens[i].value == "}")
                                {
                                    locationBraceLevel--;
                                    serverBraceLevel--;
                                    httpBraceLevel--;
                                    if (locationBraceLevel == 0)
                                    {
                                        i++; // Move past closing brace
                                        break;
                                    }
                                }

                                if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL)
                                {
                                    std::string locationDirective = tokens[i].value;
                                    i++;

                                    if (locationDirective == "root" && i < tokens.size())
                                    {
                                        location.setRoot(tokens[i].value);
                                        i++;
                                    }
                                    else if (locationDirective == "index" && i < tokens.size())
                                    {
                                        std::vector<std::string> indexFiles;
                                        while (i < tokens.size() && tokens[i].value != ";")
                                        {
                                            indexFiles.push_back(tokens[i].value);
                                            i++;
                                        }
                                        location.insertIndex(indexFiles);
                                    }
                                    else if (locationDirective == "autoindex" && i < tokens.size())
                                    {
                                        if (tokens[i].value == "on")
                                        {
                                            location.activateAutoIndex();
                                        }
                                        i++;
                                    }
                                }
                                else
                                {
                                    // Skip semicolons and other tokens
                                    if (tokens[i].value == ";")
                                        i++;
                                    else
                                        i++;
                                }
                            }

                            server.addLocation(location);
                        }
                    }
                }
                else
                {
                    // Skip semicolons and other tokens
                    if (tokens[i].value == ";")
                        i++;
                    else
                        i++;
                }
            }

            container.insertServer(server);
        }
        else
        {
            i++;
        }
    }

    return container;
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