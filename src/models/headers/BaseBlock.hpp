#ifndef BLOCKSERVER_HPP
#define BLOCKSERVER_HPP

#include <CommonExceptions.hpp>
#include <utils.hpp>

class BaseBlock {
  protected:
      std::string _root;
      std::pair<u_int16_t, std::string> _returnData;
      size_t _clientMaxBodySize;
      std::vector<std::string> _indexFiles;
      std::map<u_int16_t, std::string> _errorPages;
      bool _autoIndex;
      BaseBlock();
      BaseBlock(const BaseBlock &obj);
      virtual ~BaseBlock();

  public:
      void setRoot(const std::string &root);
      void setClientMaxBodySize(std::string &sSize);
      void insertIndex(const std::vector<std::string> &routes);
      void insertErrorPage(u_int16_t errorCode, const std::string &errorPage);
      void activateAutoIndex();
      const std::string &getRoot() const;
      const std::pair<u_int16_t, std::string> &getReturnData() const;
      size_t getClientMaxBodySize() const;
      const std::vector<std::string> &getIndexFiles() const;
      const std::string *getErrorPage(const u_int16_t code) const;
      bool getAutoIndex() const;
};

#endif