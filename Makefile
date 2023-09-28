# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mjuin <mjuin@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/04/22 11:56:29 by tcazenav          #+#    #+#              #
#    Updated: 2023/09/28 11:21:19 by mjuin            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRC = main.cpp\
      srcs/Channel.cpp\
      srcs/User.cpp\
      srcs/StaticFunctions.cpp\
	  srcs/Server.cpp\
	  srcs/Parser.cpp

HEAD =	headers/Channel.hpp \
		headers/Server.hpp \
		headers/StaticFunctions.hpp \
		headers/User.hpp \
		headers/Parser.hpp

CFLAGS = -Wall -Wextra -Werror -g -std=c++98 -I./headers/

OBJ = $(SRC:.cpp=.o)

%.o: %.cpp $(HEAD)
	c++ $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	 c++ $(OBJ) -o $(NAME) 

all: $(NAME)

clean:
	rm -f $(OBJ)

fclean:  clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
