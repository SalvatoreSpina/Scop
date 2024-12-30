// WindowManager.hpp
#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include "Camera.hpp"
#include "OBJModel.hpp"

constexpr int kDefaultWidth = 1920;
constexpr int kDefaultHeight = 1080;
constexpr const char* kDefaultTitle = "OpenGL Window";

struct Window {
  int width;
  int height;
  const char* title;

  Window(int width, int height, const char* title)
      : width(width), height(height), title(title) {}
  Window() : Window(kDefaultWidth, kDefaultHeight, kDefaultTitle) {}
};

class WindowManager {
 public:
  /**
   * @brief Initializes GLFW.
   * @return true if initialization succeeds, false otherwise.
   */
  static bool initializeGLFW();

  /**
   * @brief Creates a GLFW window of the given size and title.
   * @param width Window width in pixels.
   * @param height Window height in pixels.
   * @param title Title of the GLFW window.
   * @return Pointer to the created GLFW window, or nullptr on failure.
   */
  static GLFWwindow* createWindow(Window windowConfig);

  /**
   * @brief Runs the main rendering loop until the user closes the window.
   * @param window Valid pointer to a GLFW window.
   * @param model Reference to the OBJ model to render.
   */
  static void mainLoop(GLFWwindow* window, const OBJModel& model);
};
