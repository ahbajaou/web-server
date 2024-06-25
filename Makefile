NAME = webserv

SRC = srcs/main.cpp srcs/conf.cpp srcs/WebServer.cpp \
	srcs/Request.cpp srcs/Response.cpp srcs/mime_types.cpp srcs/4xx.cpp \
	srcs/5xx.cpp srcs/methods.cpp srcs/Client.cpp

FLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 -fsanitize=address -g3

$(NAME) : $(SRC)
	c++ $(FLAGS) $(SRC) -o $@

all : $(NAME)

clean :
	clear
	rm -rf $(NAME)

fclean : clean

re : fclean all