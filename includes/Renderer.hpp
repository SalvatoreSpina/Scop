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
   */
  Renderer(GLFWwindow *window, int width, int height, OBJModel &model);

  /**
   * @brief Destructor: Cleans up resources.
   */
  ~Renderer();

  /**
   * @brief Runs the main rendering loop.
   * @param model The OBJ model to re\der.
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

  // Camera controls
  void resetToDefaults();

  // Rendering modes
  void drawAllFaces(const OBJModel &model);

  // Texture loading
  GLuint loadBMPTexture(const std::string &filePath);
  void loadTextureFromFile(const std::string &filePath);
  void generateWhiteTexture(unsigned int width, unsigned int height);

  // Model loading
  void loadModelFromFile(const std::string &filePath);
  void buildFaceBasedColors(const OBJModel &_current_model);

  /**
   * @brief Updates the Overlay with the latest window size.
   */
  void updateOverlayWindowSize();

  // Free Camera Controls
  void handleFreeCameraMovement(float deltaTime);
  void handleFreeCameraRotation(float deltaTime);

private:
  // Model
  OBJModel _current_model;

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

  // Overlay
  Overlay m_overlay;

  // Camera Mode Flag
  bool m_freeCameraMode;

  // Timing for smooth movement
  double m_lastFrameTime;

  // Movement flags
  bool m_moveForward;
  bool m_moveBackward;
  bool m_moveLeft;
  bool m_moveRight;
  bool m_moveUp;
  bool m_moveDown;

  // Rotation deltas
  float m_yawDelta;
  float m_pitchDelta;

  // Transition-related members
  bool m_transitioning = false;
  bool m_fadeOut = false;
  float m_transitionAlpha = 0.0f;
  float m_transitionDuration = 0.25; // Duration of each fade phase in seconds
  float m_transitionElapsed = 0.0f;
  RenderMode m_nextMode;
  float m_lastDeltaTime = 0.0f; // Store last frame's delta time for transitions
};
