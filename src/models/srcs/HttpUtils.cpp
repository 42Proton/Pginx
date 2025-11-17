#include "HttpUtils.hpp"
#include <algorithm>
#include <cctype>
#include <fcntl.h>

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
