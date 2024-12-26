#pragma once

#include <GLFW/glfw3.h>

bool initializeGLFW();
void mainLoop(GLFWwindow *window);
GLFWwindow *createWindow(int width, int height, const char *title);