#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <ctime>
#include <sys/types.h>

struct DirEntry {
    std::string name;
    bool isDir;
    off_t size;
    time_t mtime;
    
    bool operator<(const DirEntry& other) const {
        if (isDir != other.isDir)
            return isDir > other.isDir;
        return name < other.name;
    }
};

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

// Auto indexing utilities

std::string generateAutoIndexPage(const std::string &dirPath, const std::string &requestPath);
bool compareEntries(const std::pair<std::string, struct stat> &a,
                   const std::pair<std::string, struct stat> &b);

std::string formatFileSize(off_t size);
std::string formatModTime(time_t modTime);

#endif
