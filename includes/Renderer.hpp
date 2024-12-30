#pragma once

#include "Camera.hpp"
#include "OBJModel.hpp"
#include <GLFW/glfw3.h>

class Renderer {
public:
  Renderer(GLFWwindow *window, int width, int height);
  ~Renderer();

  void run(const OBJModel &model);

private:
  void initializeGL();
  void renderFrame(const OBJModel &model);

private:
  GLFWwindow *m_window;
  int m_width;
  int m_height;

  Camera m_camera;
  float m_rotationAngle;
};