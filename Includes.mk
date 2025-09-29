SRCS=\
	main.cpp\
	utils.cpp\
	extCheck.cpp\
	parser/lexer.cpp\
	parser/readFile.cpp

MODELS=\
	BaseBlock.cpp\
	CommonExceptions.cpp\
	Server.cpp\
	Container.cpp\
	SocketManager.cpp
TEMPLATES=\

HEADERS=$(MODELS:.cpp=.hpp)