CPP = c++
FLAGS = -Wall -Werror -Wextra -std=c++98
NAME = serialize
HEADER = Serializer.hpp Data.hpp
SRC = main.cpp Serializer.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(FLAGS) $^ -o $(NAME)

%.o:%.cpp $(HEADER)
	$(CPP) $(FLAGS) -c $< -o $@

re: fclean all

fclean: clean
	rm -f $(NAME)

clean:
	rm -f $(OBJ)

.PHONY: clean fclean re all