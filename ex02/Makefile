CPP = c++
FLAGS = -Wall -Werror -Wextra -std=c++98
NAME = identify
HEADER = Base.hpp A.hpp B.hpp C.hpp
SRC = main.cpp Base.cpp A.cpp B.cpp C.cpp
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