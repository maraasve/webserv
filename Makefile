NAME = webserv
CPP = c++
CPPFLAGS = -Wall -Werror -Wextra -std=c++17 -MMD -fPIE -g3
CPPFLAGS += -fno-limit-debug-info
LDFLAGS = -lstdc++fs
RM = rm -rf
SRC_DIR = src
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/Networks/*.cpp) $(wildcard $(SRC_DIR)/Parsing/*.cpp) $(wildcard $(SRC_DIR)/Server/*.cpp) $(wildcard $(SRC_DIR)/CGI/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(OBJ_DIR) $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/Networks
	mkdir -p $(OBJ_DIR)/Parsing
	mkdir -p $(OBJ_DIR)/Server
	mkdir -p $(OBJ_DIR)/CGI


$(NAME):$(OBJS)
	$(CPP) $(OBJS) -o $(NAME) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CPP) -c $< -o $@

-include $(DEPS)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

