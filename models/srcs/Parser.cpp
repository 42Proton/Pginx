#include <Parser.hpp>

Parser::Parser(const std::string &filePath) : _filePath(filePath)
{
}

Parser::~Parser()
{
}

void Parser::parseTokinizer()
{
}

ParserBlock* Parser::parserPrepBlocks()
{
    ParserBlock* parserBlock = new ParserBlock;
    return parserBlock;
}

ServerContainer* Parser::parserProcess()
{
    ServerContainer* serverContainer = new ServerContainer();
    return serverContainer;
}

ServerContainer* Parser::parseFile()
{
    parseTokinizer();
    ParserBlock* parserBlock = parserPrepBlocks();
    ServerContainer* serverContainer = parserProcess();
    return serverContainer;
}