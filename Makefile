include Includes.mk

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -g3  -I./includes -I./templates -I./src/models/headers

MODELS_DR = src
INCLUDES_DR = includes
SRCS_DR = src
TEMPLATES_DIR= templates

TEMPLATES_S= $(addprefix $(TEMPLATES_DIR)/,$(TEMPLATES))
MODELS_DR_SRC= $(addprefix $(MODELS_DR)/,$(MODELS))
SRCS_DR_SRC= $(addprefix $(SRCS_DR)/,$(SRCS))
HEADERS_SRC= $(addprefix $(MODELS_DR)/,$(HEADERS))
MODELS_OBJS= $(MODELS_DR_SRC:%.cpp=build/%.o)
SRCS_OBJS= $(SRCS_DR_SRC:%.cpp=build/%.o)

HEADERS_SRC += $(TEMPLATES_S)
HEADERS_SRC += $(INCLUDES_DR)

NAME = webserv
all: $(NAME)

$(NAME): $(MODELS_OBJS) $(SRCS_OBJS)
	$(CXX) $(MODELS_OBJS) $(SRCS_OBJS) $(CXXFLAGS) -o $(NAME)

build/%.o:%.cpp  $(HEADERS_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -f  $(MODELS_OBJS) $(SRCS_OBJS)
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re