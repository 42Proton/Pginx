#include <Container.hpp>
#include <LocationConfig.hpp>
#include <Server.hpp>
#include <iomanip>
#include <utils.hpp>

std::vector<std::string> split(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::string current;

  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == delimiter) {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
    } else {
      current += str[i];
    }
  }

  if (!current.empty()) {
    tokens.push_back(current);
  }

  return tokens;
}

std::vector<std::string> split(const std::string& str,
                               const std::string& delimiter) {
  std::vector<std::string> result;

  if (delimiter.empty()) {
    result.push_back(str);
    return result;
  }

  size_t start = 0;
  size_t found = str.find(delimiter, start);

  while (found != std::string::npos) {
    if (found != start) {
      result.push_back(str.substr(start, found - start));
    }
    start = found + delimiter.length();
    found = str.find(delimiter, start);
  }

  if (start < str.length()) {
    result.push_back(str.substr(start));
  }

  return result;
}

const char& str_back(const std::string& str) {
  static const char nullChar = '\0';
  if (str.empty())
    return nullChar;
  return str[str.size() - 1];
}

void printQueryParams(const std::map<std::string, std::string>& queryParams) {
  std::cout << "queryParams: ";
  for (std::map<std::string, std::string>::const_iterator it =
           queryParams.begin();
       it != queryParams.end(); ++it) {
    std::cout << it->first << "=" << it->second;
    // Check if this is not the last element
    std::map<std::string, std::string>::const_iterator nextIt = it;
    ++nextIt;
    if (nextIt != queryParams.end())
      std::cout << ", ";
  }
  std::cout << std::endl;
}

// utils
bool endsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string getMimeType(const std::string& file) {
  if (endsWith(file, ".html"))
    return "text/html";
  if (endsWith(file, ".css"))
    return "text/css";
  if (endsWith(file, ".js"))
    return "application/javascript";
  if (endsWith(file, ".json"))
    return "application/json";
  if (endsWith(file, ".png"))
    return "image/png";
  if (endsWith(file, ".jpg") || endsWith(file, ".jpeg"))
    return "image/jpeg";
  return "application/octet-stream";  // fallback for unknown types
}

void printContainer(const Container& container) {
  const std::vector<Server>& servers = container.getServers();

  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "CONTAINER CONFIGURATION SUMMARY" << std::endl;
  std::cout << std::string(80, '=') << std::endl;
  std::cout << "Total Servers: " << servers.size() << std::endl;
  std::cout << std::string(80, '=') << std::endl;

  for (size_t i = 0; i < servers.size(); ++i) {
    const Server& server = servers[i];

    std::cout << "\n[Server #" << (i + 1) << "]" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // Listen directives
    const std::vector<ListenCtx>& listens = server.getListens();
    std::cout << "  Listen:" << std::endl;
    for (size_t j = 0; j < listens.size(); ++j) {
      std::cout << "    - " << listens[j].addr << ":" << listens[j].port
                << std::endl;
    }

    // Server names
    const std::vector<std::string>& serverNames = server.getServerNames();
    if (!serverNames.empty()) {
      std::cout << "  Server Names:" << std::endl;
      for (size_t j = 0; j < serverNames.size(); ++j) {
        std::cout << "    - " << serverNames[j] << std::endl;
      }
    }

    // Root
    std::cout << "  Root: " << server.getRoot() << std::endl;

    // Client max body size
    size_t maxBodySize = server.getClientMaxBodySize();
    std::cout << "  Client Max Body Size: ";
    if (maxBodySize == 0) {
      std::cout << "unlimited";
    } else if (maxBodySize >= GIGABYTE) {
      std::cout << (maxBodySize / GIGABYTE) << "G";
    } else if (maxBodySize >= MEGABYTE) {
      std::cout << (maxBodySize / MEGABYTE) << "M";
    } else if (maxBodySize >= KILOBYTE) {
      std::cout << (maxBodySize / KILOBYTE) << "K";
    } else {
      std::cout << maxBodySize << " bytes";
    }
    std::cout << std::endl;

    // Index files
    const std::vector<std::string>& indexFiles = server.getIndexFiles();
    if (!indexFiles.empty()) {
      std::cout << "  Index Files: ";
      for (size_t j = 0; j < indexFiles.size(); ++j) {
        std::cout << indexFiles[j];
        if (j < indexFiles.size() - 1)
          std::cout << ", ";
      }
      std::cout << std::endl;
    }

    // AutoIndex
    std::cout << "  AutoIndex: " << (server.getAutoIndex() ? "on" : "off")
              << std::endl;

    // Error pages
    const std::map<u_int16_t, std::string>& errorPages =
        server.getErrorPage(0)
            ? *reinterpret_cast<const std::map<u_int16_t, std::string>*>(0)
            : std::map<u_int16_t, std::string>();
    // Note: getErrorPage returns pointer to single page, not full map
    // This is a limitation - we'll just note if error pages are configured
    if (server.getErrorPage(404) != NULL) {
      std::cout << "  Error Pages: configured (404, etc.)" << std::endl;
    }

    // Locations
    const std::vector<LocationConfig>& locations = server.getLocations();
    if (!locations.empty()) {
      std::cout << "\n  Locations (" << locations.size() << "):" << std::endl;
      for (size_t j = 0; j < locations.size(); ++j) {
        const LocationConfig& loc = locations[j];

        // Display match type
        std::string matchTypeStr;
        switch (loc.getMatchType()) {
          case EXACT:
            matchTypeStr = "= ";
            break;
          case REGEX_CASE:
            matchTypeStr = "~ ";
            break;
          case REGEX_ICASE:
            matchTypeStr = "~* ";
            break;
          case PRIORITY_PREFIX:
            matchTypeStr = "^~ ";
            break;
          case NAMED:
            matchTypeStr = "@ ";
            break;
          case PREFIX:
          default:
            matchTypeStr = "";
            break;
        }

        std::cout << "\n    [Location: " << matchTypeStr << loc.getPath() << "]"
                  << std::endl;

        // Allowed methods
        const std::vector<std::string>& methods = loc.getMethods();
        if (!methods.empty()) {
          std::cout << "      Methods: ";
          for (size_t k = 0; k < methods.size(); ++k) {
            std::cout << methods[k];
            if (k < methods.size() - 1)
              std::cout << ", ";
          }
          std::cout << std::endl;
        }

        // Upload directory
        if (!loc.getUploadDir().empty()) {
          std::cout << "      Upload Dir: " << loc.getUploadDir() << std::endl;
        }

        // Root (from BaseBlock)
        if (!loc.getRoot().empty()) {
          std::cout << "      Root: " << loc.getRoot() << std::endl;
        }

        // Index files
        const std::vector<std::string>& locIndexFiles = loc.getIndexFiles();
        if (!locIndexFiles.empty()) {
          std::cout << "      Index: ";
          for (size_t k = 0; k < locIndexFiles.size(); ++k) {
            std::cout << locIndexFiles[k];
            if (k < locIndexFiles.size() - 1)
              std::cout << ", ";
          }
          std::cout << std::endl;
        }

        // AutoIndex
        std::cout << "      AutoIndex: " << (loc.getAutoIndex() ? "on" : "off")
                  << std::endl;
      }
    }

    std::cout << std::string(80, '-') << std::endl;
  }

  std::cout << "\n" << std::string(80, '=') << std::endl;
  std::cout << "END OF CONFIGURATION" << std::endl;
  std::cout << std::string(80, '=') << std::endl << std::endl;
}
