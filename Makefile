NAME = webserv
CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++11 -MD -g3
RM = rm -rf
SRC_DIR = src
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/Networks/*.cpp) $(wildcard $(SRC_DIR)/Parsing/*.cpp) $(wildcard $(SRC_DIR)/Server/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(OBJ_DIR) $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/Networks
	mkdir -p $(OBJ_DIR)/Parsing
	mkdir -p $(OBJ_DIR)/Server

$(NAME):$(OBJS)
	$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) -c $< -o $@

# -include $(DEPS)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

