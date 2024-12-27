NAME =		scop

COMP =		c++
CPPFLAGS =	-Wall -Werror -Wextra -std=c++20 -Iincludes #-g -fsanitize=address
LDFLAGS = -lGL -lglfw

SRCS =		srcs/main.cpp \
			srcs/obj_parser.cpp \
			srcs/parsing.cpp \
			srcs/window.cpp

OBJS =		$(SRCS:%.cpp=%.o)

all: $(NAME)

$(NAME):	$(OBJS)
	@$(COMP) $(CPPFLAGS) $(LDFLAGS) $^ -o $@

clean:
	@$(RM) $(OBJS)

fclean:		clean
	@$(RM) $(NAME)

re:		fclean all

sanitize:
	@$(COMP) $(CPPFLAGS) $(LDFLAGS) -g -fsanitize=address $(SRCS) -o $(NAME)

build:
	@docker build -t scop_image .

destroy:
	@docker rmi scop_image

# Target to format code using Docker
format:
	@docker run --rm -v $(PWD):/usr/src/app scop_image clang-format -i srcs/*.cpp includes/*.hpp