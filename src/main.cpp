#include <iostream>
#include <parser.hpp>
#include <utils.hpp>
#include <SocketManager.hpp>
#include <Container.hpp>
#include <Server.hpp>
#include <sstream>

std::string intToString(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
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

        // TODO: Once your parser is complete, replace this section with:
        // Container container = parser(tokens);
        // std::vector<Server> servers = container.getServers(); // You'll need to add this method
        
        // For now, create test servers manually (remove this when parser is ready)
        std::vector<Server> servers;
        
        // Create a test server based on your config
        Server testServer;
        testServer.insertListen(8080, "127.0.0.1");
        testServer.insertServerNames("localhost");
        testServer.setRoot("/usr/share/nginx/html");
        servers.push_back(testServer);
        
        // Convert servers to socket info
        std::vector<ServerSocketInfo> socketInfos = convertServersToSocketInfo(servers);
        
        // Initialize socket manager
        SocketManager socketManager;
        
        std::cout << "Initializing " << socketInfos.size() << " sockets..." << std::endl;
        
        if (socketManager.initSockets(socketInfos)) {
            std::cout << "✅ All sockets initialized successfully!" << std::endl;
            
            // Print socket details
            const std::vector<int>& sockets = socketManager.getSockets();
            for (size_t i = 0; i < sockets.size() && i < socketInfos.size(); ++i) {
                std::cout << "Socket " << i << " (fd=" << sockets[i] 
                          << ") listening on " << socketInfos[i].host 
                          << ":" << socketInfos[i].port << std::endl;
            }
            
            // TODO: Add your main server loop here
            // For now, just keep sockets open briefly for testing
            std::cout << "Press Enter to shutdown..." << std::endl;
            std::cin.get();
            
        } else {
            std::cerr << "❌ Failed to initialize sockets!" << std::endl;
            return 1;
        }

        // Print tokens (for debugging - remove when not needed)
        std::cout << "\nToken output:" << std::endl;
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
