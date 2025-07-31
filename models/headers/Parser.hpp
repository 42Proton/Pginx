#ifndef PARSER_HPP
#define PARSER_HPP

#include <ServerContainer.hpp>
#include <utils.hpp>

typedef struct tagParserEntry
{
    std::string entryName;
    std::deque<std::string> values;
} ParserEntry;

typedef struct tagParserBlock
{
    std::string blockName;
    std::deque<std::string> headerData;
    std::deque<ParserEntry> entries;
    std::deque<struct tagParserBlock *> innerBlocks;
} ParserBlock;

class Parser
{
  private:
    std::string _filePath;
    std::deque<std::string> _tokens;
    /*
    I tried to keep the parser 2 phases but dealing with ParserBlock directly can be difficult
    because of inner blocks which would require hardcoding the behaviour and especially
    inheritance handling which can be painful if we would process entries one by one
    */
    void parseTokinizer();
    ParserBlock *parserPrepBlocks();
    ServerContainer *parserProcess();

  public:
    Parser(const std::string &filePath);
    ServerContainer *parseFile();
};

#endif