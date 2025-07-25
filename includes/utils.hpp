#ifndef UTILS_HPP
#define UTILS_HPP

#include <CommonExceptions.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

// nginx implement the following compile time macro which defines the root relative path
#define PGINX_PREFIX "/var/lib/pginx/"
// default root path
#define DEFAULT_ROOT_PATH "/var/lib/pginx/html/"

// Units of measure
#define KILOBYTE 1024
#define MEGABYTE 1048576
#define GIGABYTE 1073741824


std::string initValidation(int argc, char **argv);
std::vector<std::string> split(const std::string &str, char delimiter);
std::vector<std::string> split(const std::string &str, const std::string &delimiter);
const char& str_back(const std::string& str);

#endif