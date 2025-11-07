#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>

// String utilities
std::string ltrim(const std::string& s);
std::string rtrim(const std::string& s);
std::string trim(const std::string& s);
std::string toLowerStr(const std::string& s);

// Number parsing/conversion
size_t parseHex(const std::string& s);
size_t safeAtoi(const std::string& s);
std::string itoa_custom(size_t n);
std::string itoa_int(int n);

// URL utilities
std::string urlDecode(const std::string& s);

// Socket utilities
bool setNonBlocking(int fd);

std::string extractFileName(const std::string &path);

#endif
