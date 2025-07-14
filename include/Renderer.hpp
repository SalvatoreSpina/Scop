#pragma once

#include "Camera.hpp"
#include "Matrix4.hpp"
#include "OBJModel.hpp"
#include "Overlay.hpp"
#include <GLFW/glfw3.h>

#include <array>
#include <string>
#include <vector>

/**
 * @brief Handles OpenGL rendering, user input, and interaction.
 */
class Renderer {
public:
  /**
   * @brief Constructs a Renderer.
   * @param window Pointer to the GLFW window.
   * @param width Initial window width.
   * @param height Initial window height.
   * @param model Reference to the initial OBJ model.
   */
  Renderer(GLFWwindow *window, int width, int height, OBJModel &model);

  /**
   * @brief Destructor: Cleans up resources.
   */
  ~Renderer();

  /**
   * @brief Runs the main rendering loop.
   */
  void run();

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

  static void mouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  void onMouseButton(int button, int action, int mods);

  void resetToDefaults();
  void drawAllFaces(const OBJModel &model);

  void loadTextureFromFile(const std::string &filePath);
  void loadModelFromFile(const std::string &filePath);
  void buildFaceBasedColors(const OBJModel &model);

  void handleFreeCameraMovement(float deltaTime);
  void handleFreeCameraRotation(float deltaTime);

private:
  // Model
  OBJModel currentModel_;

  GLFWwindow *window_;
  int width_;
  int height_;

  // Camera and rotation
  Camera camera_;
  float rotationAngle_;
  float rotationSpeed_;

  Matrix4 modelTranslation_;

  // Default camera settings for reset
  Vector3 defaultEye_;
  Vector3 defaultCenter_;
  Vector3 defaultUp_;
  float defaultFovy_;
  float defaultRotationSpeed_;

  RenderMode currentRenderMode_;

  std::vector<std::array<float, 3>> faceGrayColors_;
  std::vector<std::array<float, 3>> faceRandomColors_;
  std::vector<std::array<float, 3>> faceMaterialColors_;

  GLuint textureID_;

  Overlay overlay_;

  bool isFreeCameraMode_;

  double lastFrameTime_;

  bool moveForward_;
  bool moveBackward_;
  bool moveLeft_;
  bool moveRight_;
  bool moveUp_;
  bool moveDown_;

  float yawDelta_;
  float pitchDelta_;

  // Transition-related members for smooth mode changes
  bool transitioning_;
  bool fadeOut_;
  float transitionAlpha_;
  float transitionDuration_;
  float transitionElapsed_;
  RenderMode nextRenderMode_;
  float lastDeltaTime_;

  // Only for special flip
  float nextFlipAngle_;
};
