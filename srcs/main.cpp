#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "scop.hpp"

struct OBJModel;

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <path/to/your/model.obj>\n";
}

bool parsing(int argc, char** argv, OBJModel& model)
{
    if (argc != 2) {
        printUsage(argv[0]);
        return false;
    }

    std::string filePath = argv[1];
    std::cerr << "Loading OBJ file: " << filePath << "\n";

    if (loadOBJ(filePath, model)) {
        std::cout << "Loaded OBJ file successfully!\n";
        std::cout << "Vertices: " << model.vertices.size() << "\n";
        std::cout << "Texture Coords: " << model.texCoords.size() << "\n";
        std::cout << "Normals: " << model.normals.size() << "\n";
        std::cout << "Faces: " << model.faces.size() << "\n";
        return true;
    } else {
        std::cerr << "Failed to load OBJ file.\n";
        return false;
    }
}

bool initializeGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return false;
    }
    return true;
}

GLFWwindow* createWindow(int width, int height, const char* title) {
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
    }
    return window;
}

void mainLoop(GLFWwindow* window) {
    while (!glfwWindowShouldClose(window)) {
        // Render here

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }
}

int main(int argc, char** argv) {


    OBJModel model;

    if (!parsing(argc, argv, model))
        return 1;

    GLFWwindow* window = createWindow(1920, 1080, "Scop");
    if (!window) {
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!initializeGLFW()) {
        return 1;
    }

    mainLoop(window);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}