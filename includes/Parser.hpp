#ifndef PARSER_HPP
#define PARSER_HPP

#include <Server.hpp>
#include <utils.hpp>

class Parser
{
    private:
        std::vector<Server> _servers;
    public:
        Parser(const std::string &filePath);
        ~Parser();
        const std::vector<Server> &getServers() const;
        void validateServers() const;
};

#endif