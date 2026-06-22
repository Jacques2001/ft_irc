NAME        = ircserv
OBJ_DIR     = objs

RED     = \033[0;31m
GREEN   = \033[0;32m
YELLOW  = \033[0;33m
BLUE    = \033[0;34m
CYAN    = \033[0;36m
RESET   = \033[0m

SRCS        =   main.cpp \
                srcs/Client.cpp \
                srcs/Server.cpp \
				srcs/Channel.cpp

OBJS        = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

CC          = c++ -g -std=c++98
CFLAGS      = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJS) includes/Server.hpp includes/Client.hpp includes/Channel.hpp
	@printf "$(CYAN)🔧 Linking: $(NAME)$(RESET)\n"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN)✅ Compilation terminée avec succès !$(RESET)\n"

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)📝 Compiling: $<$(RESET)\n"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@printf "$(YELLOW)🧹 Cleaning objects...$(RESET)\n"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@printf "$(RED)🗑️  Full clean...$(RESET)\n"
	@rm -f $(NAME)

re: fclean all
	@printf "$(GREEN)♻️  Rebuilding project...$(RESET)\n"

.PHONY: all clean fclean re