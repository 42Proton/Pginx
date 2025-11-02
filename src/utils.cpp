#include <utils.hpp>

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == delimiter)
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += str[i];
        }
    }

    if (!current.empty())
    {
        tokens.push_back(current);
    }

    return tokens;
}

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> result;

    if (delimiter.empty())
    {
        result.push_back(str);
        return result;
    }

    size_t start = 0;
    size_t found = str.find(delimiter, start);

    while (found != std::string::npos)
    {
        if (found != start)
        {
            result.push_back(str.substr(start, found - start));
        }
        start = found + delimiter.length();
        found = str.find(delimiter, start);
    }

    if (start < str.length())
    {
        result.push_back(str.substr(start));
    }

    return result;
}

const char &str_back(const std::string &str)
{
    static const char nullChar = '\0';
    if (str.empty())
        return nullChar;
    return str[str.size() - 1];
}

void printQueryParams(const std::map<std::string, std::string>& queryParams) {
    std::cout << "queryParams: ";
    for (std::map<std::string, std::string>::const_iterator it = queryParams.begin(); it != queryParams.end(); ++it) {
        std::cout << it->first << "=" << it->second;
        // Check if this is not the last element
        std::map<std::string, std::string>::const_iterator nextIt = it;
        ++nextIt;
        if (nextIt != queryParams.end())
            std::cout << ", ";
    }
    std::cout << std::endl;
}

//utils
bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string getMimeType(const std::string &file) {
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
    return "application/octet-stream"; // fallback for unknown types
}
