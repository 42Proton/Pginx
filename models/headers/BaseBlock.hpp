#ifndef BLOCKSERVER_HPP
#define BLOCKSERVER_HPP

#include <utils.hpp>

class BaseBlock
{
    protected:
        std::string _root;
        std::pair<u_int16_t, std::string> _returnData;
        size_t _clientMaxBodySize;
        std::vector<std::string> _indexFiles;
        std::map<u_int16_t, const std::string*> _errorPages;
        std::set<std::string> _errorPagesCache;
        bool _autoIndex;
        BaseBlock();
        BaseBlock(const BaseBlock& obj);
        virtual ~BaseBlock() {};
    public:
        void setRoot(const std::string& root);
        void setReturnData(
            const u_int16_t code,
            const std::string& route = ""
        );
        void setClientMaxBodySize(std::string& sSize);
        void insertIndex(const std::vector<std::string>& routes);
        void insertErrorPage(
            const std::vector<u_int16_t>& errorCodes,
            const std::string& errorPage
        );
        void activateAutoIndex();
        const std::string& getRoot() const;
        const std::pair<u_int16_t, std::string>& getReturnData() const;
        size_t getClientMaxBodySize() const;
        const std::string getIndex() const;
        const std::string getErrorPage(const u_int16_t code) const;
        bool getAutoIndex() const;
};

#endif