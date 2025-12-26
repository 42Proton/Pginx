#include "HttpUtils.hpp"
#include <algorithm>
#include <cctype>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

std::string ltrim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    return s.substr(start);
}

std::string rtrim(const std::string& s) {
    size_t end = s.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(0, end);
}

std::string trim(const std::string& s) {
    return ltrim(rtrim(s));
}

std::string toLowerStr(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    }
    return result;
}

size_t parseHex(const std::string& s) {
    size_t val = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c >= '0' && c <= '9') val = val * 16 + (c - '0');
        else if (c >= 'a' && c <= 'f') val = val * 16 + (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val = val * 16 + (c - 'A' + 10);
        else break;
    }
    return val;
}

size_t safeAtoi(const std::string& s) {
    size_t val = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] >= '0' && s[i] <= '9') {
            val = val * 10 + (s[i] - '0');
        } else break;
    }
    return val;
}

std::string itoa_custom(size_t n) {
    if (n == 0) return "0";
    std::string s;
    while (n > 0) {
        s.push_back('0' + (n % 10));
        n /= 10;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

std::string itoa_int(int n) {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    std::string s;
    while (n > 0) {
        s.push_back('0' + (n % 10));
        n /= 10;
    }
    if (neg) s.push_back('-');
    std::reverse(s.begin(), s.end());
    return s;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::string urlDecode(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int h1 = hexval(s[i + 1]);
            int h2 = hexval(s[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                result.push_back(static_cast<char>((h1 << 4) | h2));
                i += 2;
                continue;
            }
        } else if (s[i] == '+') {
            result.push_back(' ');
            continue;
        }
        result.push_back(s[i]);
    }
    return result;
}

bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

std::string extractFileName(const std::string &path) {
    if (path.empty())
        return "";
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
        return path;
    return path.substr(pos + 1);
}


std::string formatFileSize(off_t size) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    size_t suffixIndex = 0;
    double displaySize = static_cast<double>(size);
    while (displaySize >= 1024 && suffixIndex < 4) {
        displaySize /= 1024;
        ++suffixIndex;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << displaySize << " " << suffixes[suffixIndex];
    return oss.str();
}

std::string generateAutoIndexPage(const std::string &dirPath, const std::string &requestPath) {
    DIR *dir = opendir(dirPath.c_str());
    if (!dir) {
        return "<html><body><h1>Error reading directory</h1></body></html>";
    }

    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "<title>Index of " << requestPath << "</title>\n"
         << "<style>\n"
         << "body { font-family: monospace; margin: 20px; }\n"
         << "h1 { border-bottom: 1px solid #ccc; }\n"
         << "table { border-collapse: collapse; width: 100%; }\n"
         << "th { text-align: left; padding: 8px; border-bottom: 2px solid #ddd; }\n"
         << "td { padding: 8px; border-bottom: 1px solid #eee; }\n"
         << "a { text-decoration: none; color: #0066cc; }\n"
         << "a:hover { text-decoration: underline; }\n"
         << "</style>\n"
         << "</head>\n"
         << "<body>\n"
         << "<h1>Index of " << requestPath << "</h1>\n"
         << "<table>\n"
         << "<tr><th>Name</th><th>Size</th><th>Date Modified</th></tr>\n";

    // Add parent directory link if not root
    if (requestPath != "/") {
        html << "<tr><td><a href=\"../\">../</a></td><td>-</td><td>-</td></tr>\n";
    }

    struct dirent *entry;
    std::vector<std::pair<std::string, struct stat> > entries;

    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        
        if (name[0] == '.') {
            continue;
        }

        std::string fullPath = dirPath;
        if (fullPath.empty() || fullPath[fullPath.size() - 1] != '/') {
            fullPath += '/';
        }
        fullPath += name;

        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            entries.push_back(std::make_pair(name, fileStat));
        }
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end(), compareEntries);


    for (size_t i = 0; i < entries.size(); ++i) {
        const std::string &name = entries[i].first;
        const struct stat &fileStat = entries[i].second;

        bool isDir = S_ISDIR(fileStat.st_mode);
        std::string displayName = name;
        if (isDir) {
            displayName += "/";
        }

        std::string linkPath = requestPath;
        if (linkPath.empty() || linkPath[linkPath.size() - 1] != '/') {
            linkPath += '/';
        }
        linkPath += name;
        if (isDir) {
            linkPath += "/";
        }

        std::string sizeStr;
        if (isDir) {
            sizeStr = "-";
        } else {
            sizeStr = formatFileSize(fileStat.st_size);
        }

        // Format date
        char dateStr[64];
        struct tm *timeInfo = localtime(&fileStat.st_mtime);
        strftime(dateStr, sizeof(dateStr), "%d-%b-%Y %H:%M", timeInfo);

        html << "<tr>"
             << "<td><a href=\"" << linkPath << "\">" << displayName << "</a></td>"
             << "<td>" << sizeStr << "</td>"
             << "<td>" << dateStr << "</td>"
             << "</tr>\n";
    }

    html << "</table>\n"
         << "</body>\n"
         << "</html>\n";

    return html.str();
}

// Helper function to compare directory entries
bool compareEntries(const std::pair<std::string, struct stat> &a,
                   const std::pair<std::string, struct stat> &b) {
    bool aIsDir = S_ISDIR(a.second.st_mode);
    bool bIsDir = S_ISDIR(b.second.st_mode);
    
    if (aIsDir != bIsDir) {
        return aIsDir;
    }
    
    std::string aLower = a.first;
    std::string bLower = b.first;
    for (size_t i = 0; i < aLower.length(); ++i) {
        aLower[i] = std::tolower(aLower[i]);
    }
    for (size_t i = 0; i < bLower.length(); ++i) {
        bLower[i] = std::tolower(bLower[i]);
    }
    
    return aLower < bLower;
}
