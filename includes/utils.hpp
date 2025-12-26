#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>
#include <unistd.h>
#include <CommonExceptions.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

// Forward declarations
class Container;
class Server;
class LocationConfig;

// nginx implement the following compile time macro which defines the root
// relative path
#define PGINX_PREFIX "/var/lib/pginx/"
// default root path
#define DEFAULT_ROOT_PATH "/var/lib/pginx/html/"

// Units of measure
#define KILOBYTE 1024
#define MEGABYTE 1048576
#define GIGABYTE 1073741824
// Units of measure limits
#define MAX_KILOBYTE 18014398509481984UL
#define MAX_MEGABYTE 17592186044416UL
#define MAX_GIGABYTE 17179869184UL

#define PORT 4269
#define DEFAULT_PATH "config/default.conf"
#define MAX_EXT_LENGTH 30
#define MAX_TOKENS_LENGTH 8192
#define MAX_TOKEN_COUNT 102400

std::string initValidation(int argc, char** argv);
std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> split(const std::string& str,
                               const std::string& delimiter);
const char& str_back(const std::string& str);

std::string getMimeType(const std::string& file);
bool endsWith(const std::string& str, const std::string& suffix);
void printQueryParams(const std::map<std::string, std::string>& queryParams);

// Configuration printing
void printContainer(const Container& container);

#endif