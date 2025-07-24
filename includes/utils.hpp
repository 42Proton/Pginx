#ifndef UTILS_HPP
#define UTILS_HPP

#include <CommonExceptions.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

std::string initValidation(int argc, char **argv);
std::vector<std::string> split(const std::string &str, char delimiter);
std::vector<std::string> split(const std::string &str, const std::string &delimiter);

#endif