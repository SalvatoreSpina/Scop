#include "window.hpp"

#include <iostream>

/**
 * @brief Initializes GLFW.
 *
 * @return true  If initialization succeeds.
 * @return false If initialization fails.
 */
bool initializeGLFW() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW.\n";
    return false;
  }
  return true;
}

/**
 * @brief Creates a GLFW window of the given size and title.
 *
 * @param width  Window width in pixels.
 * @param height Window height in pixels.
 * @param title  Title of the GLFW window.
 * @return GLFWwindow* Pointer to the created window, or nullptr on failure.
 */
GLFWwindow *createWindow(int width, int height, const char *title) {
  GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window.\n";
    glfwTerminate();
  }
  return window;
}

/**
 * @brief Runs the main rendering loop until the user closes the window.
 *
 * @param window Valid pointer to a GLFW window.
 */
void mainLoop(GLFWwindow *window) {
  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // TODO: Render

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }
}