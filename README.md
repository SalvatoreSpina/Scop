# Scop

Scop is a lightweight OpenGL viewer used to render Wavefront `.obj` models. It comes with a tiny OBJ parser, basic camera controls and texture support. The project was initially written for a graphics assignment and is kept intentionally small.

## Features

- Custom `.obj` parser implemented in C++20
- Wireframe, grayscale and textured rendering modes
- Simple camera navigation with keyboard and mouse
- Predefined sample models under the `objs/` directory

## Building

The project relies on `GLEW`, `GLFW` and `GLUT`. The Makefile can automatically download and build GLEW if it is missing.

```bash
# build the viewer
make

# format the code (requires the docker image from `make build`)
make format
```

The resulting executable is `scop`.

## Running

Run the viewer by passing the path to an `.obj` file and optionally a texture:

```bash
./scop path/to/model.obj path/to/texture.bmp
```

If the texture is omitted, a white texture is applied. Several example models are provided in `objs/texturized` and `objs/resources`.

## Project Structure

```
include/    - public header files
src/        - C++ source files including `main.cpp`
objs/       - example models and textures
libs/       - third party libraries (GLEW will be placed here after `make`)
Makefile    - build script
```

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.
