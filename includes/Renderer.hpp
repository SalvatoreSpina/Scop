#pragma once

#include <GL/freeglut.h> // For glutBitmapCharacter (overlay text)
#include <GLFW/glfw3.h>

#include "Camera.hpp"
#include "Matrix4.hpp"
#include "OBJModel.hpp"

#include <array>
#include <string>
#include <vector>

/**
 * @brief Different ways to color or texture the 3D model.
 */
enum class RenderMode {
  GRAYSCALE = 0,
  RANDOM_COLOR,
  MATERIAL_COLOR,
  TEXTURE,
  COUNT // Not a mode, just to help us cycle
};

class Renderer {
public:
  Renderer(GLFWwindow *window, int width, int height);
  ~Renderer();

  void run(const OBJModel &model);

private:
  // Core OpenGL initialization and rendering
  void initializeGL();
  void computeModelCenter(const OBJModel &model);
  void renderFrame(const OBJModel &model);

  // OpenGL Callbacks
  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);
  void onScroll(double xoffset, double yoffset);

  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  void onKey(int key, int scancode, int action, int mods);

  static void dropCallback(GLFWwindow *window, int count, const char **paths);
  void onDrop(int count, const char **paths);

  // Camera controls
  void resetToDefaults();

  // Overlay (HUD)
  void renderOverlay();
  void drawText(float x, float y, const char *text);

  // Rendering modes
  void drawAllFaces(const OBJModel &model);
  void setFaceColor(RenderMode mode, size_t faceIndex);

  // Texture loading
  GLuint loadBMPTexture(const std::string &filePath);
  void loadTextureFromFile(const std::string &filePath);
  void generateWhiteTexture(unsigned int width, unsigned int height);

private:
  GLFWwindow *m_window;
  int m_width;
  int m_height;

  // Camera and rotation
  Camera m_camera;
  float m_rotationAngle;
  float m_rotationSpeed;

  // Model translation to center
  Matrix4 m_modelTranslation;

  // Default camera settings for reset
  Vector3 m_defaultEye;
  Vector3 m_defaultCenter;
  Vector3 m_defaultUp;
  float m_defaultFovy;
  float m_defaultRotationSpeed;

  // Current rendering mode
  RenderMode m_currentMode;

  // Face-based colors for different modes
  std::vector<std::array<float, 3>> m_faceGrayColors;
  std::vector<std::array<float, 3>> m_faceRandomColors;
  std::vector<std::array<float, 3>> m_faceMaterialColors;

  // Texture ID
  GLuint m_textureID;
};
