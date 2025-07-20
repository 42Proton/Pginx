include Includes.mk

CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -g  -I./includes -I./templates -I./models

MODLES_DR = models
INCLUDES_DR = includes
SRCS_DR = src
TEMPLATES_DIR= templates

TEMPLATES_S= $(addprefix $(TEMPLATES_DIR)/,$(TEMPLATES))
MODELS_DR_SRC= $(addprefix $(MODLES_DR)/,$(MODELS))
SRCS_DR_SRC= $(addprefix $(SRCS_DR)/,$(SRCS))
HEADRS_SRC= $(addprefix $(MODLES_DR)/,$(HEADERS))
MODELS_OBJS= $(MODELS_DR_SRC:%.cpp=build/%.o)
SRCS_OBJS= $(SRCS_DR_SRC:%.cpp=build/%.o)

HEADERS_SRC += $(TEMPLATES_S)

NAME = pginx
all: $(NAME)

$(NAME): $(MODELS_OBJS) $(SRCS_OBJS)
	$(CC) $(MODELS_OBJS) $(SRCS_OBJS) $(CFLAGS) -o $(NAME)

build/%.o:%.cpp  $(HEADRS_SRC)
	@mkdir -p $(dir $@)
	$(CC)   $(CFLAGS) -c $< -o $@

clean: 
	rm -f  $(MODELS_OBJS) $(SRCS_OBJS)
	rm -rf build

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re