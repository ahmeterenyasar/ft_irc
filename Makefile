NAME        = ircserv
BOT_NAME    = bot

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98

SRCDIR      = .
OBJDIR      = obj
BONUSDIR    = bonus

SOURCES     = main.cpp src/client.cpp src/server.cpp src/utils.cpp src/parser.cpp \
              src/channel/channel.cpp \
              src/command/pass.cpp src/command/nick.cpp src/command/user.cpp src/command/quit.cpp \
              src/command/kick.cpp src/command/invite.cpp src/command/topic.cpp src/command/mode.cpp \
              src/command/part.cpp src/command/commands.cpp src/command/join.cpp src/command/privmsg.cpp \
              src/command/notice.cpp src/command/helpers.cpp src/command/cap.cpp src/command/ping.cpp \
              src/command/who.cpp src/command/list.cpp 

BOT_SRC     = bonus/bot.cpp

OBJECTS     = $(SOURCES:%.cpp=$(OBJDIR)/%.o)

INCLUDES    = -I.

RED         = \033[0;31m
GREEN       = \033[0;32m
YELLOW      = \033[0;33m
BLUE        = \033[0;34m
MAGENTA     = \033[0;35m
CYAN        = \033[0;36m
WHITE       = \033[0;37m
RESET       = \033[0m

all: $(NAME)

$(NAME): $(OBJECTS)
	@echo "$(GREEN)Linking $(NAME)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created successfully!$(RESET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(BLUE)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

bonus: $(NAME) $(BOT_NAME)

$(BOT_NAME): $(BOT_SRC)
	@echo "$(MAGENTA)Compiling bonus bot...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(BOT_SRC) -o $(BOT_NAME)
	@echo "$(MAGENTA)$(BOT_NAME) created successfully!$(RESET)"

clean:
	@echo "$(YELLOW)Cleaning object files...$(RESET)"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@echo "$(RED)Removing $(BOT_NAME)...$(RESET)"
	@rm -f $(BOT_NAME)

re: fclean all

.PHONY: all clean fclean re bonus