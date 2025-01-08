#include "Renderer.hpp"
#include "MeshRenderer.hpp"
#include "ModelUtils.hpp"
#include "OBJLoader.hpp"
#include "TextureManager.hpp"

#include <array>
#include <cfloat> // for FLT_MAX
#include <cmath>  // for cosf, sinf
#include <cstdio> // for snprintf
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

/**
 * @brief Constructor: Initializes OpenGL, sets up camera, loads default
 * texture.
 */
Renderer::Renderer(GLFWwindow *window, int width, int height, OBJModel &model)
    : m_window(window), m_width(width), m_height(height), m_rotationAngle(0.0f),
      m_rotationSpeed(0.5f), m_currentMode(RenderMode::GRAYSCALE),
      m_textureID(0), m_overlay(width, height), m_freeCameraMode(false),
      m_lastFrameTime(0.0), m_moveForward(false), m_moveBackward(false),
      m_moveLeft(false), m_moveRight(false), m_moveUp(false), m_moveDown(false),
      m_yawDelta(0.0f), m_pitchDelta(0.0f) {
  if (!m_window) {
    throw std::runtime_error("Renderer received a null GLFWwindow*!");
  }

  glfwMakeContextCurrent(m_window);
  initializeGL();

  // Setup default camera
  m_camera.eye = Vector3(0.0f, 0.0f, 5.0f);
  m_camera.center = Vector3(0.0f, 0.0f, 0.0f);
  m_camera.up = Vector3(0.0f, 1.0f, 0.0f);

  m_camera.fovy = 45.0f;
  m_camera.aspectRatio =
      static_cast<float>(m_width) / static_cast<float>(m_height);
  m_camera.nearZ = 0.1f;
  m_camera.farZ = 100.0f;

  // Model translation
  m_modelTranslation.setIdentity();

  // Save defaults for reset
  m_defaultEye = m_camera.eye;
  m_defaultCenter = m_camera.center;
  m_defaultUp = m_camera.up;
  m_defaultFovy = m_camera.fovy;
  m_defaultRotationSpeed = m_rotationSpeed;

  // Set up callbacks
  glfwSetWindowUserPointer(m_window, this);
  glfwSetScrollCallback(m_window, Renderer::scrollCallback);
  glfwSetKeyCallback(m_window, Renderer::keyCallback);
  glfwSetDropCallback(m_window, Renderer::dropCallback);

  this->_current_model = model;

  std::cout << "Loading texture from file: " << this->_current_model.textureName
            << std::endl;
  loadTextureFromFile(this->_current_model.textureName);
}

/**
 * @brief Destructor: Cleans up the texture.
 */
Renderer::~Renderer() {
  if (m_textureID != 0) {
    glDeleteTextures(1, &m_textureID);
  }
}

/**
 * @brief Initializes basic OpenGL settings.
 */
void Renderer::initializeGL() {
  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Set viewport
  glViewport(0, 0, m_width, m_height);

  // Set clear color
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  // Enable textures; we'll bind/unbind as needed
  glEnable(GL_TEXTURE_2D);

  // Better perspective correction for textures
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

/**
 * @brief Runs the main rendering loop.
 */
void Renderer::run() {

  auto &&model = this->_current_model;

  // Center the model
  computeModelCenter(model);

  // Save the current model
  this->_current_model = model;

  // Build face colors
  buildFaceBasedColors(model);

  // Initialize timing
  m_lastFrameTime = glfwGetTime();

  // Main rendering loop
  while (!glfwWindowShouldClose(m_window)) {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
    m_lastDeltaTime = deltaTime; // Store for transition updates
    m_lastFrameTime = currentTime;

    // Handle free camera movement
    if (m_freeCameraMode) {
      handleFreeCameraMovement(deltaTime);
      handleFreeCameraRotation(deltaTime);
    }

    renderFrame(_current_model);
    glfwSwapBuffers(m_window);
    glfwPollEvents();
  }
}

/**
 * @brief Renders a single frame with automatic uniform scaling to fit a desired
 * size.
 */
void Renderer::renderFrame(const OBJModel &model) {
  // Clear buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Setup projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Matrix4 proj = m_camera.getProjectionMatrix();
  glLoadMatrixf(proj.m);

  // Setup modelview
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Determine view matrix based on camera mode
  Matrix4 view;
  if (m_freeCameraMode) {
    view = m_camera.getViewMatrix(true); // Free Camera Mode
  } else {
    view = m_camera.getViewMatrix(false); // Focus Mode
  }
  glLoadMatrixf(view.m);

  // ----------------------------------------------------------------------
  // 1. Compute bounding box of the OBJModel
  //    (We do it here, but you could do it once when model is loaded)
  // ----------------------------------------------------------------------
  if (!model.vertices.empty()) {
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;

    for (const auto &v : model.vertices) {
      if (v.x < minX)
        minX = v.x;
      if (v.x > maxX)
        maxX = v.x;
      if (v.y < minY)
        minY = v.y;
      if (v.y > maxY)
        maxY = v.y;
      if (v.z < minZ)
        minZ = v.z;
      if (v.z > maxZ)
        maxZ = v.z;
    }

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float extent = std::max({sizeX, sizeY, sizeZ});

    // ----------------------------------------------------------------------
    // 2. Decide how large you want the model to be. For instance, if we want
    //    the longest side (X, Y, or Z) to be 2.0 units, do:
    // ----------------------------------------------------------------------
    float desiredSize = 2.0f; // or whatever size you want
    // Avoid divide-by-zero:
    float scaleFactor = (extent > 1e-5f) ? (desiredSize / extent) : 1.0f;

    // ----------------------------------------------------------------------
    // 3. Build a uniform scale matrix using the computed scaleFactor
    // ----------------------------------------------------------------------
    Matrix4 scale;
    scale.setIdentity();
    scale.m[0] = scaleFactor;  // x-scale
    scale.m[5] = scaleFactor;  // y-scale
    scale.m[10] = scaleFactor; // z-scale

    // ----------------------------------------------------------------------
    // 4. We also want to center the model around (0,0,0) or your custom center
    //    The typical approach is to translate the bounding box center to
    //    origin. We can do that with m_modelTranslation, which you might have
    //    set up in computeModelCenter(...).
    // ----------------------------------------------------------------------

    // (Optional) If you already have m_modelTranslation shifting the model
    // so its center is at origin, no extra step is needed.

    // ----------------------------------------------------------------------
    // 5. If in Focus Mode, apply rotation to spin the model
    // ----------------------------------------------------------------------
    Matrix4 rotation;
    rotation.setIdentity();
    if (!m_freeCameraMode) {
      m_rotationAngle += m_rotationSpeed;
      float radians = m_rotationAngle * 3.1415926535f / 180.0f;
      rotation.m[0] = cosf(radians);
      rotation.m[2] = sinf(radians);
      rotation.m[8] = -sinf(radians);
      rotation.m[10] = cosf(radians);
    }

    // Combine: scale -> translation -> rotation
    // (You might prefer a different order, but typically:
    //  scale * translation means "translate after scaling the model".)
    // So we do: scaledModel = rotation * (scale * m_modelTranslation)
    Matrix4 scaled = Matrix4::multiply(scale, m_modelTranslation);
    Matrix4 modelMatrix = Matrix4::multiply(rotation, scaled);

    // Multiply with the view
    Matrix4 modelView = Matrix4::multiply(view, modelMatrix);
    glLoadMatrixf(modelView.m);
  } else {
    // If no vertices, just load the view matrix
    glLoadMatrixf(view.m);
  }

  // Draw the model
  drawAllFaces(model);

  // Overlay black quad for fade transition if needed
  if (m_transitioning) {
    // Update transition timing
    m_transitionElapsed += m_lastDeltaTime;
    float progress = m_transitionElapsed / m_transitionDuration;
    if (progress > 1.0f)
      progress = 1.0f;

    if (m_fadeOut) {
      m_transitionAlpha = progress;
      if (progress >= 1.0f) {
        // Fade-out complete: switch mode and start fade-in
        m_currentMode = m_nextMode;
        m_fadeOut = false;
        m_transitionElapsed = 0.0f; // Reset for fade-in
      }
    } else {
      m_transitionAlpha = 1.0f - progress;
      if (progress >= 1.0f) {
        // Fade-in complete
        m_transitioning = false;
        m_transitionAlpha = 0.0f;
      }
    }

    // Save current matrices and attributes
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, m_width, 0, m_height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw a full-screen quad with black color and variable alpha
    glColor4f(0.0f, 0.0f, 0.0f, m_transitionAlpha);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(m_width, 0);
    glVertex2f(m_width, m_height);
    glVertex2f(0, m_height);
    glEnd();

    // Restore previous matrices and states
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
  }

  // ----------------------------------------------------------------------
  // Overlay / camera info
  // ----------------------------------------------------------------------
  std::stringstream cameraInfoStream;

  cameraInfoStream << std::fixed << std::setprecision(2) << "Camera Eye: ("
                   << m_camera.eye.x << ", " << m_camera.eye.y << ", "
                   << m_camera.eye.z << ")\n"
                   << "Camera Center: (" << m_camera.center.x << ", "
                   << m_camera.center.y << ", " << m_camera.center.z << ")\n"
                   << "Camera Up: (" << m_camera.up.x << ", " << m_camera.up.y
                   << ", " << m_camera.up.z << ")\n"
                   << "FOV: " << m_camera.fovy << " deg\n"
                   << "Rotation Speed: " << m_rotationSpeed << " deg/frame";

  std::string cameraInfo = cameraInfoStream.str();

  // Render overlay
  m_overlay.render(cameraInfo, static_cast<int>(m_currentMode),
                   static_cast<int>(RenderMode::COUNT), model);
  glfwSetMouseButtonCallback(m_window, Renderer::mouseButtonCallback);
}

/**
 * @brief Draws all faces of the model based on the current rendering mode.
 */
void Renderer::drawAllFaces(const OBJModel &model) {
  MeshRenderer::drawAllFaces(model, static_cast<RenderMode>(m_currentMode),
                             m_textureID, m_faceGrayColors, m_faceRandomColors,
                             m_faceMaterialColors);
}

/**
 * @brief Centers the model based on its bounding box.
 */
void Renderer::computeModelCenter(const OBJModel &model) {
  if (model.vertices.empty()) {
    return;
  }
  float cx, cy, cz;
  ModelUtilities::computeModelCenter(model, cx, cy, cz);
  m_modelTranslation.setIdentity();
  m_modelTranslation.m[12] = -cx;
  m_modelTranslation.m[13] = -cy;
  m_modelTranslation.m[14] = -cz;
}

void Renderer::buildFaceBasedColors(const OBJModel &model) {
  ModelUtilities::buildFaceBasedColors(
      model, m_faceGrayColors, m_faceRandomColors, m_faceMaterialColors);
}

/**
 * @brief GLFW scroll callback.
 */
void Renderer::scrollCallback(GLFWwindow *window, double xoffset,
                              double yoffset) {
  auto *r = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (r) {
    r->onScroll(xoffset, yoffset);
  }
}

/**
 * @brief Handles scroll events to adjust the camera's FOV.
 */
void Renderer::onScroll(double /*xoffset*/, double yoffset) {
  m_camera.fovy -= static_cast<float>(yoffset);
  if (m_camera.fovy < 1.0f)
    m_camera.fovy = 1.0f;
  if (m_camera.fovy > 150.0f)
    m_camera.fovy = 150.0f;
}

/**
 * @brief GLFW key callback.
 */
void Renderer::keyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  auto *r = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (r) {
    r->onKey(key, scancode, action, mods);
  }
}

/**
 * @brief Handles key events for camera movement, rotation speed, mode
 * switching, and reset.
 */
void Renderer::onKey(int key, int scancode, int action, int mods) {
  static_cast<void>(scancode); // Unused
  static_cast<void>(mods);     // Unused
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (key) {
    // Toggle Camera Mode
    case GLFW_KEY_F:
      m_freeCameraMode = !m_freeCameraMode;
      if (m_freeCameraMode) {
        std::cout << "Switched to Free Camera Mode.\n";
      } else {
        std::cout << "Switched to Focus Mode.\n";
        // Reset camera to focus on the model
        resetToDefaults();
      }
      break;

    // Rotation speed controls (only in Focus Mode)
    case GLFW_KEY_KP_ADD:
    case GLFW_KEY_EQUAL:
      if (!m_freeCameraMode)
        m_rotationSpeed += 0.1f;
      break;
    case GLFW_KEY_KP_SUBTRACT:
    case GLFW_KEY_MINUS:
      if (!m_freeCameraMode) {
        m_rotationSpeed -= 0.1f;
        if (m_rotationSpeed < 0.0f) {
          m_rotationSpeed = 0.0f;
        }
      }
      break;

    // Reset camera and rotation (only in Focus Mode)
    case GLFW_KEY_SPACE:
      if (!m_freeCameraMode)
        resetToDefaults();
      break;

      // Cycle rendering modes
    case GLFW_KEY_T: {
      if (!m_transitioning) {
        int next = static_cast<int>(m_currentMode) + 1;
        if (next >= static_cast<int>(RenderMode::COUNT)) {
          next = 0; // Wrap around
        }
        m_nextMode = static_cast<RenderMode>(next);
        // Start transition
        m_transitioning = true;
        m_fadeOut = true;
        m_transitionElapsed = 0.0f;
        m_transitionAlpha = 0.0f;
      }
      break;
    }

    // Movement controls (only in Free Camera Mode)
    case GLFW_KEY_W:
      if (m_freeCameraMode)
        m_moveForward = true;
      break;
    case GLFW_KEY_S:
      if (m_freeCameraMode)
        m_moveBackward = true;
      break;
    case GLFW_KEY_A:
      if (m_freeCameraMode)
        m_moveLeft = true;
      break;
    case GLFW_KEY_D:
      if (m_freeCameraMode)
        m_moveRight = true;
      break;
    case GLFW_KEY_Q:
      if (m_freeCameraMode)
        m_moveDown = true;
      break;
    case GLFW_KEY_E:
      if (m_freeCameraMode)
        m_moveUp = true;
      break;

    // Rotation controls (only in Free Camera Mode)
    case GLFW_KEY_LEFT:
      if (m_freeCameraMode)
        m_yawDelta = -1.0f; // Rotate left
      break;
    case GLFW_KEY_RIGHT:
      if (m_freeCameraMode)
        m_yawDelta = 1.0f; // Rotate right
      break;
    case GLFW_KEY_UP:
      if (m_freeCameraMode)
        m_pitchDelta = 1.0f; // Rotate up
      break;
    case GLFW_KEY_DOWN:
      if (m_freeCameraMode)
        m_pitchDelta = -1.0f; // Rotate down
      break;

    default:
      break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
    // Movement controls (only in Free Camera Mode)
    case GLFW_KEY_W:
      if (m_freeCameraMode)
        m_moveForward = false;
      break;
    case GLFW_KEY_S:
      if (m_freeCameraMode)
        m_moveBackward = false;
      break;
    case GLFW_KEY_A:
      if (m_freeCameraMode)
        m_moveLeft = false;
      break;
    case GLFW_KEY_D:
      if (m_freeCameraMode)
        m_moveRight = false;
      break;
    case GLFW_KEY_Q:
      if (m_freeCameraMode)
        m_moveDown = false;
      break;
    case GLFW_KEY_E:
      if (m_freeCameraMode)
        m_moveUp = false;
      break;

    // Rotation controls (only in Free Camera Mode)
    case GLFW_KEY_LEFT:
    case GLFW_KEY_RIGHT:
      if (m_freeCameraMode)
        m_yawDelta = 0.0f;
      break;
    case GLFW_KEY_UP:
    case GLFW_KEY_DOWN:
      if (m_freeCameraMode)
        m_pitchDelta = 0.0f;
      break;

    default:
      break;
    }
  }
}

/**
 * @brief Resets the camera and rotation speed to default values.
 */
void Renderer::resetToDefaults() {
  m_camera.eye = m_defaultEye;
  m_camera.center = m_defaultCenter;
  m_camera.up = m_defaultUp;
  m_camera.fovy = m_defaultFovy;
  m_rotationAngle = 0.0f;
  m_rotationSpeed = m_defaultRotationSpeed;
}

/**
 * @brief GLFW drop callback.
 */
void Renderer::dropCallback(GLFWwindow *window, int count, const char **paths) {
  auto *r = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (r) {
    r->onDrop(count, paths);
  }
}

/**
 * @brief Handles file drop events to load new textures or models.
 */
void Renderer::onDrop(int count, const char **paths) {
  if (count <= 0) {
    return;
  }
  std::string droppedFile = paths[0];
  std::cout << "Dropped file: " << droppedFile << std::endl;

  // Check file extension
  if (droppedFile.find(".bmp") != std::string::npos) {
    loadTextureFromFile(droppedFile);
    _current_model.textureName = droppedFile;
  } else if (droppedFile.find(".obj") != std::string::npos) {
    loadModelFromFile(droppedFile);
    _current_model.objectName = droppedFile;
  }
}

/**
 * @brief Loads a BMP texture from file and sets it as the current texture.
 *        Falls back to a white texture if loading fails.
 */
void Renderer::loadTextureFromFile(const std::string &filePath) {
  std::cout << "Attempting to load texture: " << filePath << std::endl;

  // Use the TextureManager to load the texture
  GLuint newTexture = TextureManager::loadBMPTexture(filePath);

  if (newTexture == 0) {
    std::cerr
        << "Failed to load texture. Generating a white texture as fallback.\n";
    newTexture = TextureManager::generateWhiteTexture(64, 64);
  }

  // Delete the old texture if it exists
  if (m_textureID != 0) {
    glDeleteTextures(1, &m_textureID);
  }
  m_textureID = newTexture;

  std::cout << "Texture loaded successfully. Texture ID: " << m_textureID
            << std::endl;
}

void Renderer::loadModelFromFile(const std::string &filePath) {
  std::cout << "Attempting to load model: " << filePath << std::endl;

  OBJModel new_model;
  bool isNewModelValid = OBJLoader::loadOBJ(filePath, new_model);
  if (isNewModelValid) {
    std::cout << "Model loaded successfully.\n";
    computeModelCenter(new_model);
    buildFaceBasedColors(new_model);
    new_model.objectName = filePath;
    new_model.textureName = this->_current_model.textureName;
    this->_current_model = new_model;
  } else {
    std::cerr << "Failed to load model.\n";
  }
}

/**
 * @brief Handles movement inputs for Free Camera Mode.
 * @param deltaTime Time elapsed since last frame (for smooth movement).
 */
void Renderer::handleFreeCameraMovement(float deltaTime) {
  float speed = 5.0f * deltaTime; // Adjust speed as necessary

  if (m_moveForward)
    m_camera.moveForward(speed);
  if (m_moveBackward)
    m_camera.moveForward(-speed);
  if (m_moveRight)
    m_camera.moveRight(speed);
  if (m_moveLeft)
    m_camera.moveRight(-speed);
  if (m_moveUp)
    m_camera.moveUp(speed);
  if (m_moveDown)
    m_camera.moveUp(-speed);
}

/**
 * @brief Handles rotation inputs for Free Camera Mode.
 * @param deltaTime Time elapsed since last frame (for smooth rotation).
 */
void Renderer::handleFreeCameraRotation(float deltaTime) {
  float rotationSpeed = 90.0f * deltaTime; // Degrees per second

  m_camera.rotate(m_yawDelta * rotationSpeed, m_pitchDelta * rotationSpeed);

  // Reset deltas after applying
  m_yawDelta = 0.0f;
  m_pitchDelta = 0.0f;
}

void Renderer::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                   int mods) {
  auto *r = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (r) {
    r->onMouseButton(button, action, mods);
  }
}

void Renderer::onMouseButton(int button, int action, int mods) {
  static_cast<void>(mods); // Unused
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double rawMouseX, rawMouseY;
    glfwGetCursorPos(m_window, &rawMouseX, &rawMouseY);

    // If overlay’s 0,0 is at bottom-left, invert Y:
    float mouseX = static_cast<float>(rawMouseX);
    float clickY = static_cast<float>(m_height) - static_cast<float>(rawMouseY);

    // For "Open Folder" text, let's guess bounding box = 120 wide, 30 tall
    float openW = 200.0f;
    float openH = 100.0f;
    float openX = static_cast<float>(m_width) - openW - 10.0f;
    float openY = 50.0f;

    if (mouseX >= openX && mouseX <= (openX + openW) && clickY >= openY &&
        clickY <= (openY + openH)) {
      // OS-specific command:
#ifdef _WIN32
      system("explorer .\\objs");
#elif __APPLE__
      system("open ./objs");
#else
      system("xdg-open ./objs");
#endif
    }
  }
}
