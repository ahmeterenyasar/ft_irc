
NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

SRCDIR		= .
OBJDIR		= obj

SOURCES		= main.cpp src/client.cpp src/server.cpp src/debug.cpp src/utils.cpp src/parser/parser.cpp \
			  src/command/pass.cpp src/command/nick.cpp src/command/user.cpp src/command/quit.cpp \
			  src/command/kick.cpp src/command/invite.cpp src/command/topic.cpp src/command/mode.cpp \
			  src/command/part.cpp src/command/commands.cpp src/command/join.cpp src/command/privmsg.cpp \
			  src/command/notice.cpp 
OBJECTS		= $(SOURCES:%.cpp=$(OBJDIR)/%.o)

INCLUDES	= -I.

RED			= \033[0;31m
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
MAGENTA		= \033[0;35m
CYAN		= \033[0;36m
WHITE		= \033[0;37m
RESET		= \033[0m

all: $(NAME)

$(NAME): $(OBJECTS)
	@echo "$(GREEN)Linking $(NAME)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created successfully!$(RESET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re