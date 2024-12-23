NAME =		abstract_vm

COMP =		c++
CPPFLAGS =	-Wall -Werror -Wextra -std=c++20 -Iincludes #-g -fsanitize=address

SRCS =		srcs/main.cpp

OBJS =		$(SRCS:%.cpp=%.o)

all: $(NAME)

$(NAME):	$(OBJS)
	@$(COMP) $(CPPFLAGS) $^ -o $@

clean:
	@$(RM) $(OBJS)

fclean:		clean
	@$(RM) $(NAME)

re:	
	@make fclean
	@make all

sanitize:
	@$(COMP) $(CPPFLAGS) -g -fsanitize=address $(SRCS) -o $(NAME)

build:
	@docker build -t abstract_vm_image .

destroy:
	@docker rmi abstract_vm_image


# Target to format code using Docker
format:
	@docker run --rm -v $(PWD):/usr/src/app abstract_vm_image clang-format -i srcs/*.cpp includes/*.hpp