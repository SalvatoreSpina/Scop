#pragma once

#include "Camera.hpp"
#include "OBJModel.hpp"
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

class Renderer {
public:
  Renderer(GLFWwindow *window, int width, int height);
  ~Renderer();

  void run(const OBJModel &model);

private:
  void initializeGL();
  void renderFrame(const OBJModel &model);

  // bounding box center
  void computeModelCenter(const OBJModel &model);

  // scroll callback for zoom
  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);
  void onScroll(double xoffset, double yoffset);

  // **Key callback** for camera movement, speed, reset
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  void onKey(int key, int scancode, int action, int mods);

  // Reset to defaults (camera position, rotation speed, etc.)
  void resetToDefaults();

  void renderOverlay();
  void drawText(float x, float y, const char *text);

private:
  GLFWwindow *m_window;
  int m_width;
  int m_height;

  Camera m_camera;
  float m_rotationAngle;

  // **Rotation speed** (degrees per frame, for instance)
  float m_rotationSpeed;

  // A matrix that translates the model to its center
  Matrix4 m_modelTranslation;

  // Store default camera, speed, etc., so we can revert on Spacebar
  Vector3 m_defaultEye;
  Vector3 m_defaultCenter;
  Vector3 m_defaultUp;
  float m_defaultFovy;
  float m_defaultRotationSpeed;
};
