NAME        := scop

CXX         := g++
CXXFLAGS    := -Wall -Werror -Wextra -std=c++20 -Iincludes -Ilibs/glew/include -fsanitize=address -g
SANFLAGS    := -fsanitize=address -g

LDFLAGS     := -Llibs/glew/lib64 -lGLEW -lGL -lglut -lglfw -Wl,-rpath,libs/glew/lib64

SRCS        := main.cpp \
			   srcs/ArgumentParser.cpp \
			   srcs/OBJLoader.cpp \
			   srcs/Window.cpp \
			   srcs/Renderer.cpp \
			   srcs/Camera.cpp \
			   srcs/Overlay.cpp \
			   srcs/TextureManager.cpp \
			   srcs/ModelUtils.cpp \
			   srcs/MashRenderer.cpp \

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

build:
	@docker build -t scop_image .

destroy:
	@docker rmi scop_image

format:
	@docker run --rm -v $(PWD):/usr/src/app scop_image clang-format -i srcs/*.cpp includes/*.hpp

grassblock: all
	./scop objs/texturized/grassblock.obj objs/texturized/grassblock.bmp

parrot: all
	./scop objs/texturized/parrot.obj objs/texturized/parrot.bmp

camel: all
	./scop objs/texturized/camel.obj objs/texturized/camel.bmp

wolf: all
	./scop objs/texturized/wolf.obj objs/texturized/wolf.bmp

trex: all
	./scop objs/texturized/trex.obj objs/texturized/trex.bmp

pikachu: all
	./scop objs/texturized/pikachu.obj objs/texturized/pikachu.bmp

world: all
	./scop objs/texturized/world.obj objs/texturized/world.bmp

nice: all
	./scop objs/basic/42nice.obj 