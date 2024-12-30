NAME        := scop

CXX         := g++
CXXFLAGS    := -Wall -Werror -Wextra -std=c++20 -Iincludes -Ilibs/glew/include
SANFLAGS    := -fsanitize=address -g

LDFLAGS     := -Llibs/glew/lib64 -lGLEW -lGL -lglfw -Wl,-rpath,libs/glew/lib64

SRCS        := main.cpp \
               srcs/ArgumentParser.cpp \
               srcs/OBJLoader.cpp \
               srcs/window.cpp \
               srcs/Renderer.cpp

OBJS        := $(SRCS:.cpp=.o)

GLEW_URL    := https://sourceforge.net/projects/glew/files/glew/2.2.0/glew-2.2.0.tgz
GLEW_DIR    := libs/glew
GLEW_TGZ    := glew.tgz

.PHONY: all clean fclean re glew sanitize

all: $(NAME)

$(NAME): glew $(OBJS)
	@$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) -o $@
	@echo "Linking complete -> $(NAME)"

glew:
	@mkdir -p $(GLEW_DIR)
	@if [ ! -d "$(GLEW_DIR)/include" ]; then \
		echo "Downloading and building GLEW..."; \
		curl -L $(GLEW_URL) -o $(GLEW_TGZ); \
		mkdir -p $(GLEW_DIR)/src; \
		tar -xzf $(GLEW_TGZ) --strip-components=1 -C $(GLEW_DIR)/src; \
		rm $(GLEW_TGZ); \
		cd $(GLEW_DIR)/src && make GLEW_DEST=.. install; \
	fi

%.o: %.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Removing object files..."
	@rm -f $(OBJS)

fclean: clean
	@echo "Removing final binary..."
	@rm -f $(NAME)

re: fclean all

%.sanitize.o: %.cpp
	@echo "Compiling $< with Address Sanitizer..."
	@$(CXX) $(CXXFLAGS) $(SANFLAGS) -c $< -o $@

build:
	@docker build -t scop_image .

destroy:
	@docker rmi scop_image

format:
	@docker run --rm -v $(PWD):/usr/src/app scop_image clang-format -i srcs/*.cpp includes/*.hpp