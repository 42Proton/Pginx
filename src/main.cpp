#include <Container.hpp>
#include <iostream>
#include <parser.hpp>
#include <utils.hpp>

void printContainer(const Container &container)
{
    std::cout << "\n=== PARSED CONFIGURATION ===" << std::endl;
    std::cout << "Number of servers: " << container.getServers().size() << std::endl;

    for (size_t i = 0; i < container.getServers().size(); ++i)
    {
        const Server &server = container.getServers()[i];
        std::cout << "\n--- Server " << (i + 1) << " ---" << std::endl;

        // Print listen addresses
        const std::vector<ListenCtx> &listens = server.getListens();
        std::cout << "Listen addresses (" << listens.size() << "):" << std::endl;
        for (size_t j = 0; j < listens.size(); ++j)
        {
            std::cout << "  " << listens[j].addr << ":" << listens[j].port << std::endl;
        }

        // Print server names
        const std::vector<std::string> &serverNames = server.getServerNames();
        std::cout << "Server names (" << serverNames.size() << "):" << std::endl;
        for (size_t j = 0; j < serverNames.size(); ++j)
        {
            if (!serverNames[j].empty())
                std::cout << "  " << serverNames[j] << std::endl;
        }

        // Print root
        std::cout << "Root: " << server.getRoot() << std::endl;

        // Print client max body size
        std::cout << "Client max body size: " << server.getClientMaxBodySize() << " bytes" << std::endl;

        // Print auto index
        std::cout << "Auto index: " << (server.getAutoIndex() ? "on" : "off") << std::endl;

        // Print locations
        const std::vector<LocationConfig> &locations = server.getLocations();
        std::cout << "Locations (" << locations.size() << "):" << std::endl;
        for (size_t j = 0; j < locations.size(); ++j)
        {
            const LocationConfig &location = locations[j];
            std::cout << "  Location: " << location.getPath() << std::endl;
            std::cout << "    Root: " << location.getRoot() << std::endl;
            std::cout << "    Auto index: " << (location.getAutoIndex() ? "on" : "off") << std::endl;

            const std::vector<std::string> &methods = location.getMethods();
            std::cout << "    Allowed methods (" << methods.size() << "): ";
            for (size_t k = 0; k < methods.size(); ++k)
            {
                std::cout << methods[k];
                if (k < methods.size() - 1)
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "=== END CONFIGURATION ===" << std::endl;
}

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

        std::cout << "Parsing configuration..." << std::endl;
        Container container = parser(tokens);

        // Print the parsed container contents
        printContainer(container);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
