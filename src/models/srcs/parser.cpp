#include <Container.hpp>
#include <cstdlib>
#include <parser.hpp>
#include <utils.hpp>

bool expect(std::string expected, Token token) {
  return token.value == expected;
}

static size_t parseLocationDirective(const std::vector<Token>& tokens,
                                     size_t i,
                                     LocationConfig& location) {
  if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
    std::string locationDirective = tokens[i].value;
    i++;

    if (locationDirective == "root" && i < tokens.size()) {
      location.setRoot(tokens[i].value);
      i++;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'root' directive");
      }
      i++;
    } else if (locationDirective == "index" && i < tokens.size()) {
      std::vector<std::string> indexFiles;
      while (i < tokens.size() && tokens[i].value != ";") {
        // Check if token is a valid index file (not another directive)
        if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
          throw std::runtime_error("Expected ';' after 'index' directive");
        }
        indexFiles.push_back(tokens[i].value);
        i++;
      }
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'index' directive");
      }
      i++;
      location.insertIndex(indexFiles);
    } else if (locationDirective == "autoindex" && i < tokens.size()) {
      if (tokens[i].value == "on") {
        location.activateAutoIndex();
      }
      i++;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'autoindex' directive");
      }
      i++;
    } else if (locationDirective == "error_page" && i < tokens.size()) {
      std::vector<u_int16_t> errorCodes;
      std::string errorPage;
      while (i < tokens.size() && tokens[i].value != ";" &&
             tokens[i].type == NUMBER) {
        errorCodes.push_back(
            static_cast<u_int16_t>(std::atoi(tokens[i].value.c_str())));
        i++;
      }

      if (i < tokens.size() && tokens[i].value != ";") {
        errorPage = tokens[i].value;
        i++;
      }

      if (!errorCodes.empty() && !errorPage.empty()) {
        for (size_t j = 0; j < errorCodes.size(); ++j) {
          location.insertErrorPage(errorCodes[j], errorPage);
        }
      }
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'error_page' directive");
      }
      i++;
    } else if (locationDirective == "upload_dir" && i < tokens.size()) {
      location.setUploadDir(tokens[i].value);
      i++;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'upload_dir' directive");
      }
      i++;
    } else if (locationDirective == "allow_methods" && i < tokens.size()) {
      std::vector<std::string> methods;
      while (i < tokens.size() && tokens[i].value != ";") {
        // Check if token is a valid method (not another directive)
        if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
          throw std::runtime_error(
              "Expected ';' after 'allow_methods' directive");
        }
        methods.push_back(tokens[i].value);
        i++;
      }
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error(
            "Expected ';' after 'allow_methods' directive");
      }
      i++;
      if (!methods.empty()) {
        location.setMethods(methods);
      }
    } else if (locationDirective == "cgi_enabled" && i < tokens.size()) {
      std::string value = tokens[i].value;
      i++;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'cgi_enabled' directive");
      }
      i++;
      if (value == "on") {
        location.setCgiEnabled(true);
      } else if (value == "off") {
        location.setCgiEnabled(false);
      } else {
        throw std::runtime_error("Invalid value for 'cgi_enabled': " + value);
      }
    } else if (locationDirective == "transfer_encoding" && i < tokens.size()) {
      std::string value = tokens[i].value;
      i++;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error(
            "Expected ';' after 'transfer_encoding' directive");
      }
      i++;
      if (value == "on") {
        location.setTransferEncoding(true);
      } else if (value == "off") {
        location.setTransferEncoding(false);
      } else {
        throw std::runtime_error("Invalid value for 'transfer_encoding': " +
                                 value);
      }
    } else if (locationDirective == "cgi_pass" && i + 1 < tokens.size()) {
      std::string extension = tokens[i].value;
      std::string interpreter = tokens[i + 1].value;
      i += 2;
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'cgi_pass' directive");
      }
      i++;
      location.setCgiPassMapping(extension, interpreter);
    } else if (locationDirective == "return" && i < tokens.size()) {
      // Parse: return <code> <url>;
      if (tokens[i].type != NUMBER) {
        throw std::runtime_error(
            "Expected status code after 'return' directive");
      }
      u_int16_t code =
          static_cast<u_int16_t>(std::atoi(tokens[i].value.c_str()));
      i++;
      std::string url;
      if (i < tokens.size() && tokens[i].value != ";") {
        url = tokens[i].value;
        i++;
      }
      if (i >= tokens.size() || tokens[i].value != ";") {
        throw std::runtime_error("Expected ';' after 'return' directive");
      }
      i++;
      location.setReturn(code, url);
    } else {
      while (i < tokens.size() && tokens[i].value != ";") {
        i++;
      }
      if (i < tokens.size() && tokens[i].value == ";") {
        i++;
      }
      throw std::runtime_error("Unknown location directive: " +
                               locationDirective);
    }
  } else {
    throw std::runtime_error("Expected location directive, got: " +
                             tokens[i].value);
  }
  // Note: semicolons are now consumed by each directive handler
  return i;
}

static size_t parseLocation(const std::vector<Token>& tokens,
                            size_t i,
                            Server& server,
                            int& serverBraceLevel,
                            int& httpBraceLevel) {
  MatchType matchType = PREFIX;
  std::string path;

  if (i >= tokens.size()) {
    throw std::runtime_error("Expected location path or modifier");
  }

  std::string firstToken = tokens[i].value;

  if (firstToken == "=") {
    matchType = EXACT;
    i++;
    if (i >= tokens.size()) {
      throw std::runtime_error("Expected path after '=' modifier");
    }
    path = tokens[i].value;
    i++;
  } else if (firstToken == "~") {
    matchType = REGEX_CASE;
    i++;
    if (i >= tokens.size()) {
      throw std::runtime_error("Expected regex pattern after '~' modifier");
    }
    path = tokens[i].value;
    i++;
  } else if (firstToken == "~*") {
    matchType = REGEX_ICASE;
    i++;
    if (i >= tokens.size()) {
      throw std::runtime_error("Expected regex pattern after '~*' modifier");
    }
    path = tokens[i].value;
    i++;
  } else if (firstToken == "^~") {
    matchType = PRIORITY_PREFIX;
    i++;
    if (i >= tokens.size()) {
      throw std::runtime_error("Expected path after '^~' modifier");
    }
    path = tokens[i].value;
    i++;
  } else if (firstToken[0] == '@') {
    matchType = NAMED;
    path = firstToken;
    i++;
  } else {
    path = firstToken;
    i++;
  }

  if (path.empty()) {
    throw std::runtime_error("Location path cannot be empty");
  }

  LocationConfig location(path, matchType);

  int locationBraceLevel = 0;
  if (i >= tokens.size() || tokens[i].value != "{") {
    throw std::runtime_error("Expected '{' after location path '" + path + "'");
  }
  locationBraceLevel++;
  serverBraceLevel++;
  httpBraceLevel++;
  i++;

  while (i < tokens.size() && locationBraceLevel > 0) {
    if (tokens[i].value == "{") {
      locationBraceLevel++;
      serverBraceLevel++;
      httpBraceLevel++;
    } else if (tokens[i].value == "}") {
      if (locationBraceLevel <= 0) {
        throw std::runtime_error("Unexpected '}' in location block");
      }
      locationBraceLevel--;
      serverBraceLevel--;
      httpBraceLevel--;
      if (locationBraceLevel == 0) {
        i++;
        break;
      }
    }

    i = parseLocationDirective(tokens, i, location);
  }

  if (locationBraceLevel != 0) {
    throw std::runtime_error("Unclosed 'location' block for '" + path +
                             "': missing '}'");
  }

  location.inheritCgiFromParent(server.isCgiEnabled());

  location.inheritCgiPassFromParent(server.getCgiPassMap());

  server.addLocation(location);
  return i;
}

static size_t parseErrorPageDirective(const std::vector<Token>& tokens,
                                      size_t i,
                                      Server& server) {
  std::vector<u_int16_t> errorCodes;
  std::string errorPage;
  while (i < tokens.size() && tokens[i].value != ";" &&
         tokens[i].type == NUMBER) {
    errorCodes.push_back(
        static_cast<u_int16_t>(std::atoi(tokens[i].value.c_str())));
    i++;
  }

  if (i < tokens.size() && tokens[i].value != ";") {
    errorPage = tokens[i].value;
    i++;
  }

  if (!errorCodes.empty() && !errorPage.empty()) {
    for (size_t j = 0; j < errorCodes.size(); ++j) {
      server.insertErrorPage(errorCodes[j], errorPage);
    }
  }

  if (i >= tokens.size() || tokens[i].value != ";") {
    throw std::runtime_error("Expected ';' after 'error_page' directive");
  }
  i++;

  return i;
}

static size_t parseIndexDirective(const std::vector<Token>& tokens,
                                  size_t i,
                                  Server& server) {
  std::vector<std::string> indexFiles;
  while (i < tokens.size() && tokens[i].value != ";") {
    // Check if token is a valid index file (not another directive)
    if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
      throw std::runtime_error("Expected ';' after 'index' directive");
    }
    indexFiles.push_back(tokens[i].value);
    i++;
  }
  if (i >= tokens.size() || tokens[i].value != ";") {
    throw std::runtime_error("Expected ';' after 'index' directive");
  }
  i++;
  server.insertIndex(indexFiles);
  return i;
}

static size_t parseBasicServerDirective(const std::vector<Token>& tokens,
                                        size_t i,
                                        Server& server,
                                        const std::string& directive) {
  if (directive == "listen" && i < tokens.size()) {
    while (i < tokens.size() && tokens[i].value != ";") {
      // Check if token is a valid listen value (not another directive)
      if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
        throw std::runtime_error("Expected ';' after 'listen' directive");
      }

      std::string listenValue = tokens[i].value;
      u_int16_t port = 80;
      std::string addr = "0.0.0.0";

      size_t colonPos = listenValue.find(':');
      if (colonPos != std::string::npos) {
        addr = listenValue.substr(0, colonPos);
        std::string portStr = listenValue.substr(colonPos + 1);
        if (!portStr.empty()) {
          port = static_cast<u_int16_t>(std::atoi(portStr.c_str()));
        }
        server.insertListen(port, addr);
      } else {
        if (!listenValue.empty()) {
          port = static_cast<u_int16_t>(std::atoi(listenValue.c_str()));
        }
        server.insertListen(port);
      }
      i++;
    }
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'listen' directive");
    }
    i++;
  } else if (directive == "server_name" && i < tokens.size()) {
    while (i < tokens.size() && tokens[i].value != ";") {
      // Check if token is a valid server name (not another directive)
      if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
        throw std::runtime_error("Expected ';' after 'server_name' directive");
      }
      server.insertServerNames(tokens[i].value);
      i++;
    }
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'server_name' directive");
    }
    i++;
  } else if (directive == "root" && i < tokens.size()) {
    server.setRoot(tokens[i].value);
    i++;
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'root' directive");
    }
    i++;
  } else if (directive == "client_max_body_size" && i < tokens.size()) {
    std::string sizeStr = tokens[i].value;
    server.setClientMaxBodySize(sizeStr);
    i++;
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error(
          "Expected ';' after 'client_max_body_size' directive");
    }
    i++;
  } else if (directive == "autoindex" && i < tokens.size()) {
    if (tokens[i].value == "on") {
      server.activateAutoIndex();
    }
    i++;
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'autoindex' directive");
    }
    i++;
  } else if (directive == "cgi_enabled" && i < tokens.size()) {
    if (tokens[i].value == "on") {
      server.setCgiEnabled(true);
    }
    i++;
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'cgi_enabled' directive");
    }
    i++;
  } else if (directive == "cgi_pass" && i + 1 < tokens.size()) {
    std::string extension = tokens[i].value;
    std::string interpreter = tokens[i + 1].value;
    i += 2;
    if (i >= tokens.size() || tokens[i].value != ";") {
      throw std::runtime_error("Expected ';' after 'cgi_pass' directive");
    }
    i++;
    server.setCgiPassMapping(extension, interpreter);
  }
  return i;
}

static size_t parseServerDirective(const std::vector<Token>& tokens,
                                   size_t i,
                                   Server& server,
                                   int& serverBraceLevel,
                                   int& httpBraceLevel) {
  if (tokens[i].type == ATTRIBUTE || tokens[i].type == LEVEL) {
    std::string directive = tokens[i].value;
    i++;

    if (directive == "index" && i < tokens.size()) {
      i = parseIndexDirective(tokens, i, server);
    } else if (directive == "error_page" && i < tokens.size()) {
      i = parseErrorPageDirective(tokens, i, server);
    } else if (directive == "location") {
      i = parseLocation(tokens, i, server, serverBraceLevel, httpBraceLevel);
    } else {
      i = parseBasicServerDirective(tokens, i, server, directive);
    }
  }
  return i;
}

static size_t parseServer(const std::vector<Token>& tokens,
                          size_t i,
                          Container& container,
                          int& httpBraceLevel) {
  Server server;
  i++;

  int serverBraceLevel = 0;
  if (i >= tokens.size() || tokens[i].value != "{") {
    throw std::runtime_error("Expected '{' after 'server'");
  }
  serverBraceLevel++;
  httpBraceLevel++;
  i++;
  while (i < tokens.size() && serverBraceLevel > 0) {
    // Track server brace levels
    if (tokens[i].value == "{") {
      serverBraceLevel++;
      httpBraceLevel++;
    } else if (tokens[i].value == "}") {
      if (serverBraceLevel <= 0) {
        throw std::runtime_error("Unexpected '}' in server block");
      }
      serverBraceLevel--;
      httpBraceLevel--;
      if (serverBraceLevel == 0) {
        i++;
        break;
      }
    }

    i = parseServerDirective(tokens, i, server, serverBraceLevel,
                             httpBraceLevel);
  }

  if (serverBraceLevel != 0) {
    throw std::runtime_error("Unclosed 'server' block: missing '}'");
  }

  container.insertServer(server);
  return i;
}

Container parser(const std::vector<Token>& tokens) {
  Container container;

  if (tokens.empty())
    throw std::runtime_error("Empty configuration");

  size_t i = 0;
  int httpBraceLevel = 0;

  // Check if config starts with 'http' block or directly with 'server' blocks
  bool hasHttpBlock = expect("http", tokens[0]);

  if (hasHttpBlock) {
    // Parse with http block wrapper
    i = 1;
    if (i >= tokens.size() || tokens[i].value != "{") {
      throw std::runtime_error("Expected '{' after 'http'");
    }
    httpBraceLevel++;
    i++;

    while (i < tokens.size() && httpBraceLevel > 0) {
      if (tokens[i].value == "{") {
        httpBraceLevel++;
      } else if (tokens[i].value == "}") {
        if (httpBraceLevel <= 0) {
          throw std::runtime_error("Unexpected '}' outside of any block");
        }
        httpBraceLevel--;
        if (httpBraceLevel == 0) {
          i++;
          break;
        }
      }

      if (tokens[i].type == LEVEL && tokens[i].value == "server") {
        i = parseServer(tokens, i, container, httpBraceLevel);
      } else {
        i++;
      }
    }

    if (httpBraceLevel != 0) {
      throw std::runtime_error("Unclosed 'http' block: missing '}'");
    }

    if (i < tokens.size()) {
      while (i < tokens.size() && tokens[i].value == ";") {
        i++;
      }
      if (i < tokens.size()) {
        throw std::runtime_error("Unexpected tokens after 'http' block");
      }
    }
  } else {
    // Parse server blocks directly without http wrapper
    while (i < tokens.size()) {
      // Skip semicolons between server blocks
      while (i < tokens.size() && tokens[i].value == ";") {
        i++;
      }

      if (i >= tokens.size()) {
        break;
      }

      if (tokens[i].type == LEVEL && tokens[i].value == "server") {
        int dummyBraceLevel = 0;  // Reset for each server block
        i = parseServer(tokens, i, container, dummyBraceLevel);
      } else {
        throw std::runtime_error("Expected 'server' block at top level");
      }
    }
  }

  if (container.getServers().empty()) {
    throw std::runtime_error("No server blocks defined in configuration");
  }

  return container;
}