Name = ircserv
SRC = main.cpp src/client.cpp src/server.cpp	
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
OBJ = $(SRC:.cpp=.o)

all: $(Name)
$(Name): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(Name) $(OBJ)
clean:
	rm -f $(OBJ)
fclean: clean
	rm -f $(Name)
re: fclean all	
.PHONY: all clean fclean re
