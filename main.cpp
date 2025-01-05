#include "ArgumentParser.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

/**
 * @brief Program entry point.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return int Exit code.
 */
int main(int argc, char **argv) {
  OBJModel model;
  glutInit(&argc, argv);

  // 1. Parse command-line arguments & load the model
  Parser argumentParser(argc, argv, model);
  if (!argumentParser.getSuccess()) {
    return EXIT_FAILURE;
  }

  // 2. Initialize GLFW
  if (!WindowManager::initializeGLFW()) {
    return EXIT_FAILURE;
  }

  // 3. Create the window
  Window window_config;
  GLFWwindow* window = WindowManager::createWindow(window_config);
  if (!window) {
    return EXIT_FAILURE;  // createWindow already prints error + terminates
  }

  // 4. Make the context current AFTER the window has been created
  glfwMakeContextCurrent(window);

  // 5. Create a Renderer using this window
  auto renderer = std::make_unique<Renderer>(window, kDefaultWidth, kDefaultHeight, model);

  // 6. Run the rendering / main loop
  renderer->run();

  // 7. Clean up
  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
