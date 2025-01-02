#include "Renderer.hpp"

#include "OBJLoader.hpp"

#include <array>
#include <cfloat> // for FLT_MAX
#include <cmath>  // for cosf, sinf
#include <cstdio> // for snprintf
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

/**
 * @brief Constructor: Initializes OpenGL, sets up camera, loads default
 * texture.
 */
Renderer::Renderer(GLFWwindow *window, int width, int height)
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

  // Load default texture
  loadTextureFromFile("textures/white.bmp");
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
}

void Renderer::buildFaceBasedColors(const OBJModel &_current_model) {
  // Build face-based colors
  std::mt19937 rng(12345);
  std::uniform_real_distribution<float> dist(0.2f, 1.0f);

  m_faceGrayColors.resize(_current_model.faces.size());
  m_faceRandomColors.resize(_current_model.faces.size());
  m_faceMaterialColors.resize(_current_model.faces.size());

  for (size_t i = 0; i < _current_model.faces.size(); ++i) {
    // Grayscale: random gray per face
    float grey = dist(rng);
    m_faceGrayColors[i] = {grey, grey, grey};

    // Random color per face
    float r = dist(rng);
    float g = dist(rng);
    float b = dist(rng);
    m_faceRandomColors[i] = {r, g, b};

    // Material color: placeholder (can be replaced with actual material
    // parsing)
    m_faceMaterialColors[i] = {0.3f, 0.6f, 1.0f}; // Example: sky blue
  }
}

/**
 * @brief Runs the main rendering loop.
 * @param model The OBJ model to render.
 */
void Renderer::run(const OBJModel &model) {
  // Center the model
  computeModelCenter(model);

  this->_current_model = model;
  buildFaceBasedColors(model);

  // Initialize timing
  m_lastFrameTime = glfwGetTime();

  // Main rendering loop
  while (!glfwWindowShouldClose(m_window)) {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
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
 * @brief Renders a single frame.
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

  // Apply rotation (only in Focus Mode)
  if (!m_freeCameraMode) {
    m_rotationAngle += m_rotationSpeed;
    float radians = m_rotationAngle * 3.1415926535f / 180.0f;
    Matrix4 rotation;
    rotation.setIdentity();
    rotation.m[0] = cosf(radians);
    rotation.m[2] = sinf(radians);
    rotation.m[8] = -sinf(radians);
    rotation.m[10] = cosf(radians);

    Matrix4 modelMatrix = Matrix4::multiply(rotation, m_modelTranslation);
    Matrix4 modelView = Matrix4::multiply(view, modelMatrix);
    glLoadMatrixf(modelView.m);
  }

  // Draw the model based on current mode
  drawAllFaces(model);

  // Prepare camera information string
  std::stringstream cameraInfoStream;
  cameraInfoStream << "Camera Eye: (" << m_camera.eye.x << ", "
                   << m_camera.eye.y << ", " << m_camera.eye.z << ")\n"
                   << "Camera Center: (" << m_camera.center.x << ", "
                   << m_camera.center.y << ", " << m_camera.center.z << ")\n"
                   << "Camera Up: (" << m_camera.up.x << ", " << m_camera.up.y
                   << ", " << m_camera.up.z << ")\n"
                   << "FOV: " << m_camera.fovy << " deg\n"
                   << "Rotation Speed: " << m_rotationSpeed << " deg/frame";

  std::string cameraInfo = cameraInfoStream.str();

  // Render overlay
  m_overlay.render(cameraInfo, static_cast<int>(m_currentMode),
                   static_cast<int>(RenderMode::COUNT));
}

/**
 * @brief Draws all faces of the model based on the current rendering mode.
 */
void Renderer::drawAllFaces(const OBJModel &model) {
  if (m_currentMode == RenderMode::TEXTURE && m_textureID != 0) {
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glEnable(GL_TEXTURE_2D);
  } else {
    // Disable texturing
    glDisable(GL_TEXTURE_2D);
  }

  // Iterate through each face
  for (size_t faceIndex = 0; faceIndex < model.faces.size(); ++faceIndex) {
    glBegin(GL_POLYGON);

    // Set the color for the entire face
    setFaceColor(m_currentMode, faceIndex);

    // Iterate through each vertex in the face
    for (const auto &fv : model.faces[faceIndex].vertices) {
      // Validate vertex index
      if (fv.vertexIndex < 0 ||
          fv.vertexIndex >= static_cast<int>(model.vertices.size())) {
        continue; // Skip invalid indices
      }

      // If in TEXTURE mode and valid texture coordinate exists
      if (m_currentMode == RenderMode::TEXTURE && fv.texCoordIndex >= 0 &&
          fv.texCoordIndex < static_cast<int>(model.texCoords.size())) {
        const auto &tc = model.texCoords[fv.texCoordIndex];
        glTexCoord2f(tc.u, tc.v);
      }

      // Set the vertex position
      const auto &v = model.vertices[fv.vertexIndex];
      glVertex3f(v.x, v.y, v.z);
    }

    glEnd();
  }
}

/**
 * @brief Sets the face color based on the current rendering mode.
 */
void Renderer::setFaceColor(RenderMode mode, size_t faceIndex) {
  switch (mode) {
  case RenderMode::GRAYSCALE: {
    auto &gc = m_faceGrayColors[faceIndex];
    glColor3f(gc[0], gc[1], gc[2]);
    break;
  }
  case RenderMode::RANDOM_COLOR: {
    auto &rc = m_faceRandomColors[faceIndex];
    glColor3f(rc[0], rc[1], rc[2]);
    break;
  }
  case RenderMode::TEXTURE: {
    // Set to white to display the texture without tinting
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  }
  default: {
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  }
  }
}

/**
 * @brief Centers the model based on its bounding box.
 */
void Renderer::computeModelCenter(const OBJModel &model) {
  if (model.vertices.empty()) {
    return;
  }

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

  float cx = 0.5f * (minX + maxX);
  float cy = 0.5f * (minY + maxY);
  float cz = 0.5f * (minZ + maxZ);

  m_modelTranslation.setIdentity();
  m_modelTranslation.m[12] = -cx;
  m_modelTranslation.m[13] = -cy;
  m_modelTranslation.m[14] = -cz;
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
  if (m_camera.fovy > 120.0f)
    m_camera.fovy = 120.0f;
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
      int next = static_cast<int>(m_currentMode) + 1;
      if (next >= static_cast<int>(RenderMode::COUNT)) {
        next = 0; // Wrap around
      }
      m_currentMode = static_cast<RenderMode>(next);
      std::cout << "Switched mode to " << next << std::endl;
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
 * @brief Handles file drop events to load new textures.
 */
void Renderer::onDrop(int count, const char **paths) {
  if (count <= 0) {
    return;
  }
  std::string droppedFile = paths[0];
  std::cout << "Dropped file: " << droppedFile << std::endl;
  if (droppedFile.find(".bmp") != std::string::npos) {
    loadTextureFromFile(droppedFile);
  } else if (droppedFile.find(".obj") != std::string::npos) {
    loadModelFromFile(droppedFile);
  }
}

/**
 * @brief Loads a BMP texture from file and sets it as the current texture.
 *        Falls back to a white texture if loading fails.
 */
void Renderer::loadTextureFromFile(const std::string &filePath) {
  std::cout << "Attempting to load texture: " << filePath << std::endl;

  GLuint newTexture = loadBMPTexture(filePath);
  if (newTexture == 0) {
    std::cerr
        << "Failed to load texture. Generating a white texture as fallback.\n";
    generateWhiteTexture(64, 64);
  } else {
    // Delete the old texture if it exists
    if (m_textureID != 0) {
      glDeleteTextures(1, &m_textureID);
    }
    m_textureID = newTexture;
    std::cout << "Texture loaded successfully. Texture ID: " << m_textureID
              << std::endl;
  }
}

void Renderer::loadModelFromFile(const std::string &filePath) {
  std::cout << "Attempting to load model: " << filePath << std::endl;

  auto new_model = OBJModel();
  bool isNewModelValid = OBJLoader::loadOBJ(filePath, new_model);
  if (isNewModelValid) {
    std::cout << "Model loaded successfully.\n";
    computeModelCenter(new_model);
    buildFaceBasedColors(new_model);
    this->_current_model = new_model;
  } else {
    std::cerr << "Failed to load model.\n";
  }
}

/**
 * @brief Manually loads a 24-bit uncompressed BMP file and creates an OpenGL
 * texture.
 * @param filePath Path to the BMP file.
 * @return GLuint texture ID (0 if failure).
 */
GLuint Renderer::loadBMPTexture(const std::string &filePath) {
  // Open the BMP file
  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open BMP file: " << filePath << std::endl;
    return 0;
  }

  // Read the BMP header (14 bytes)
  unsigned char fileHeader[14];
  file.read(reinterpret_cast<char *>(fileHeader), 14);
  if (file.gcount() != 14) {
    std::cerr << "Error: Failed to read BMP file header.\n";
    return 0;
  }

  // Verify the BMP signature
  if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    std::cerr << "Error: File is not a valid BMP: " << filePath << std::endl;
    return 0;
  }

  // Read the DIB header (BITMAPINFOHEADER - 40 bytes)
  unsigned char infoHeader[40];
  file.read(reinterpret_cast<char *>(infoHeader), 40);
  if (file.gcount() != 40) {
    std::cerr << "Error: Failed to read BMP info header.\n";
    return 0;
  }

  // Extract image dimensions and properties
  int width = *reinterpret_cast<int *>(&infoHeader[4]);
  int height = *reinterpret_cast<int *>(&infoHeader[8]);
  short planes = *reinterpret_cast<short *>(&infoHeader[12]);
  short bpp = *reinterpret_cast<short *>(&infoHeader[14]);
  int compression = *reinterpret_cast<int *>(&infoHeader[16]);

  // Validate BMP format
  if (planes != 1) {
    std::cerr << "Error: BMP planes (" << planes << ") is not equal to 1.\n";
    return 0;
  }
  if (bpp != 24) {
    std::cerr << "Error: BMP bit depth (" << bpp << ") is not 24.\n";
    return 0;
  }
  if (compression != 0) {
    std::cerr << "Error: BMP compression (" << compression
              << ") is not supported.\n";
    return 0;
  }

  // Check if height is negative (top-down BMP)
  bool bottomUp = true;
  if (height < 0) {
    bottomUp = false;
    height = -height;
    std::cerr << "Warning: BMP is top-down. Processing as bottom-up.\n";
  }

  // Calculate row padding
  int rowSize =
      (width * 3 + 3) & (~3); // Each row is padded to a multiple of 4 bytes

  // Move to the pixel data offset (from file header)
  int dataOffset = *reinterpret_cast<int *>(&fileHeader[10]);
  file.seekg(dataOffset, std::ios::beg);

  // Read pixel data
  std::vector<unsigned char> bmpData(rowSize * height);
  file.read(reinterpret_cast<char *>(bmpData.data()), bmpData.size());
  if (file.gcount() != static_cast<std::streamsize>(bmpData.size())) {
    std::cerr << "Error: Failed to read BMP pixel data.\n";
    return 0;
  }
  file.close();

  // Convert BMP data (BGR) to RGB and flip vertically if needed
  std::vector<unsigned char> pixels(width * height * 3, 0);
  for (int y = 0; y < height; ++y) {
    int bmpY = bottomUp ? (height - 1 - y) : y;
    for (int x = 0; x < width; ++x) {
      int bmpIndex = y * rowSize + x * 3;
      int pixelIndex = (bmpY * width + x) * 3;

      // BMP stores pixels in BGR format
      pixels[pixelIndex + 0] = bmpData[bmpIndex + 2]; // R
      pixels[pixelIndex + 1] = bmpData[bmpIndex + 1]; // G
      pixels[pixelIndex + 2] = bmpData[bmpIndex + 0]; // B
    }
  }

  // Generate OpenGL texture
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data to GPU
  glTexImage2D(GL_TEXTURE_2D,
               0,                // mipmap level
               GL_RGB,           // internal format
               width, height,    // width and height
               0,                // border
               GL_RGB,           // format
               GL_UNSIGNED_BYTE, // data type
               pixels.data());   // pointer to data

  // Optionally generate mipmaps
  // glGenerateMipmap(GL_TEXTURE_2D);

  std::cout << "Loaded BMP texture: " << filePath << " (Width: " << width
            << ", Height: " << height << ", Texture ID: " << texID << ")\n";

  return texID;
}

/**
 * @brief Generates a solid white texture as a fallback.
 */
void Renderer::generateWhiteTexture(unsigned int width, unsigned int height) {
  // Delete existing texture if any
  if (m_textureID != 0) {
    glDeleteTextures(1, &m_textureID);
    m_textureID = 0;
  }

  // Create a white pixel array
  std::vector<unsigned char> whitePixels(width * height * 3,
                                         255); // RGB all 255

  // Generate OpenGL texture
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data to GPU
  glTexImage2D(GL_TEXTURE_2D,
               0,                   // mipmap level
               GL_RGB,              // internal format
               width, height,       // width and height
               0,                   // border
               GL_RGB,              // format
               GL_UNSIGNED_BYTE,    // data type
               whitePixels.data()); // pointer to data

  std::cout << "Generated a solid white texture (Texture ID: " << m_textureID
            << ", Size: " << width << "x" << height << ")\n";
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