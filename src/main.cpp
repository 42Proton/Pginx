#include <cstring>
#include <iostream>
#include "Container.hpp"
#include "SocketManager.hpp"
#include "parser.hpp"
#include "utils.hpp"

std::vector<ServerSocketInfo> convertServersToSocketInfo(
    const std::vector<Server>& servers);

int main(int argc, char** argv) {
  // if no config file found ->> load default built-in confing and print
  //"Warning: No config file provided. Using default configuration."
  if (argc != 2) {
    std::cerr << "Provide a configuration file!" << std::endl;
    return 1;
  }

  try {
    // Checkpoint 1: Initialization and configuration file reading
    initValidation(argc, argv);
    std::string content = readFile(argv[1]);
    std::vector<Token> tokens = lexer(content);
    checks(tokens);
    Container container = parser(tokens);

    // Print configuration summary
    printContainer(container);

        // // Checkpoint 2: Convert servers to socket information
    // std::vector<ServerSocketInfo> socketInfos =
    //     convertServersToSocketInfo(container.getServers());

    // // Checkpoint 3: Initialize SocketManager and start handling clients
    // SocketManager socketManager;
    // socketManager.setServers(container.getServers());

    // if (!socketManager.initSockets(socketInfos))
    //   throw std::runtime_error("Failed to initialize sockets.");

    // // Check
    // std::cout << "Server initialized. Waiting for clients..." << std::endl;
    // socketManager.handleClients();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
