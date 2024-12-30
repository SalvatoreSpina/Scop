#pragma once

#include <scop.hpp>

bool initializeGLFW();
void mainLoop(GLFWwindow *window, const OBJModel &model);
GLFWwindow *createWindow(int width, int height, const char *title);