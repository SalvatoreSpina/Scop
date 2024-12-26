#include "parsing.hpp"
#include "window.hpp" // Contains initializeGLFW(), createWindow(), and mainLoop()

#include <iostream>
#include <string>

/**
 * @brief Program entry point.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return int Exit code.
 */
int main(int argc, char **argv) {
  OBJModel model;

  // 1. Parse command-line arguments and load the model
  if (!parseArguments(argc, argv, model)) {
    return 1;
  }

  // 2. Initialize GLFW
  if (!initializeGLFW()) {
    return 1;
  }

  // 3. Create the window
  GLFWwindow *window = createWindow(960, 540, "Scop");
  if (!window) {
    return 1; // createWindow() already printed the error and called
              // glfwTerminate()
  }

  // 4. Make the context current AFTER the window has been created
  glfwMakeContextCurrent(window);

  // 5. Start the rendering/main loop
  mainLoop(window);

  // 6. Clean up GLFW
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
