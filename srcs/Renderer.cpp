#include "Renderer.hpp"
#include "MeshRenderer.hpp"
#include "ModelUtils.hpp"
#include "OBJLoader.hpp"
#include "TextureManager.hpp"

#include <array>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

Renderer::Renderer(GLFWwindow *window, int width, int height, OBJModel &model)
    : window_(window), width_(width), height_(height), rotationAngle_(0.0f),
      rotationSpeed_(0.5f), currentRenderMode_(RenderMode::GRAYSCALE),
      textureID_(0), overlay_(width, height), isFreeCameraMode_(false),
      lastFrameTime_(0.0), moveForward_(false), moveBackward_(false),
      moveLeft_(false), moveRight_(false), moveUp_(false), moveDown_(false),
      yawDelta_(0.0f), pitchDelta_(0.0f), transitioning_(false),
      fadeOut_(false), transitionAlpha_(0.0f), transitionDuration_(0.25f),
      transitionElapsed_(0.0f), lastDeltaTime_(0.0f) {
  if (!window_) {
    throw std::runtime_error("Renderer received a null GLFWwindow*!");
  }

  glfwMakeContextCurrent(window_);
  initializeGL();

  // Setup default camera
  camera_.eye = Vector3(0.0f, 0.0f, 5.0f);
  camera_.center = Vector3(0.0f, 0.0f, 0.0f);
  camera_.up = Vector3(0.0f, 1.0f, 0.0f);
  camera_.fovy = 45.0f;
  camera_.aspectRatio =
      static_cast<float>(width_) / static_cast<float>(height_);
  camera_.nearZ = 0.1f;
  camera_.farZ = 100.0f;

  modelTranslation_.setIdentity();

  // Save defaults for reset
  defaultEye_ = camera_.eye;
  defaultCenter_ = camera_.center;
  defaultUp_ = camera_.up;
  defaultFovy_ = camera_.fovy;
  defaultRotationSpeed_ = rotationSpeed_;

  // Set up GLFW callbacks
  glfwSetWindowUserPointer(window_, this);
  glfwSetScrollCallback(window_, Renderer::scrollCallback);
  glfwSetKeyCallback(window_, Renderer::keyCallback);
  glfwSetDropCallback(window_, Renderer::dropCallback);

  currentModel_ = model;

  std::cout << "Loading texture from file: " << currentModel_.textureName
            << std::endl;
  loadTextureFromFile(currentModel_.textureName);
}

Renderer::~Renderer() {
  if (textureID_ != 0) {
    glDeleteTextures(1, &textureID_);
  }
}

void Renderer::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glViewport(0, 0, width_, height_);
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

  glEnable(GL_TEXTURE_2D);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void Renderer::run() {
  auto &model = currentModel_;

  computeModelCenter(model);
  buildFaceBasedColors(model);

  lastFrameTime_ = glfwGetTime();

  while (!glfwWindowShouldClose(window_)) {
    double currentTime = glfwGetTime();
    float deltaTime = static_cast<float>(currentTime - lastFrameTime_);
    lastDeltaTime_ = deltaTime;
    lastFrameTime_ = currentTime;

    if (isFreeCameraMode_) {
      handleFreeCameraMovement(deltaTime);
      handleFreeCameraRotation(deltaTime);
    }

    renderFrame(currentModel_);
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

void Renderer::renderFrame(const OBJModel &model) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Setup projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Matrix4 projectionMatrix = camera_.getProjectionMatrix();
  glLoadMatrixf(projectionMatrix.m);

  // Setup modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Matrix4 viewMatrix = isFreeCameraMode_ ? camera_.getViewMatrix(true)
                                         : camera_.getViewMatrix(false);
  glLoadMatrixf(viewMatrix.m);

  // Compute model transformation and draw model
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
    float desiredSize = 2.0f;
    float scaleFactor = (extent > 1e-5f) ? (desiredSize / extent) : 1.0f;

    Matrix4 scale;
    scale.setIdentity();
    scale.m[0] = scaleFactor;
    scale.m[5] = scaleFactor;
    scale.m[10] = scaleFactor;

    Matrix4 rotation;
    rotation.setIdentity();
    if (!isFreeCameraMode_) {
      rotationAngle_ += rotationSpeed_;
      float radians = rotationAngle_ * 3.1415926535f / 180.0f;
      rotation.m[0] = cosf(radians);
      rotation.m[2] = sinf(radians);
      rotation.m[8] = -sinf(radians);
      rotation.m[10] = cosf(radians);
    }

    Matrix4 scaledMatrix = Matrix4::multiply(scale, modelTranslation_);
    Matrix4 modelMatrix = Matrix4::multiply(rotation, scaledMatrix);
    Matrix4 modelViewMatrix = Matrix4::multiply(viewMatrix, modelMatrix);

    glLoadMatrixf(modelViewMatrix.m);
  } else {
    glLoadMatrixf(viewMatrix.m);
  }

  drawAllFaces(model);

  // Handle fade transition overlay if transitioning
  if (transitioning_) {
    transitionElapsed_ += lastDeltaTime_;
    float progress = transitionElapsed_ / transitionDuration_;
    if (progress > 1.0f)
      progress = 1.0f;

    if (fadeOut_) {
      transitionAlpha_ = progress;
      if (progress >= 1.0f) {
        currentRenderMode_ = nextRenderMode_;
        fadeOut_ = false;
        transitionElapsed_ = 0.0f;
      }
    } else {
      transitionAlpha_ = 1.0f - progress;
      if (progress >= 1.0f) {
        transitioning_ = false;
        transitionAlpha_ = 0.0f;
      }
    }

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width_, 0, height_, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.0f, 0.0f, 0.0f, transitionAlpha_);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(width_, 0);
    glVertex2f(width_, height_);
    glVertex2f(0, height_);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
  }

  // Render camera and mode info overlay
  std::stringstream cameraInfoStream;
  cameraInfoStream << std::fixed << std::setprecision(2) << "Camera Eye: ("
                   << camera_.eye.x << ", " << camera_.eye.y << ", "
                   << camera_.eye.z << ")\n"
                   << "Camera Center: (" << camera_.center.x << ", "
                   << camera_.center.y << ", " << camera_.center.z << ")\n"
                   << "Camera Up: (" << camera_.up.x << ", " << camera_.up.y
                   << ", " << camera_.up.z << ")\n"
                   << "FOV: " << camera_.fovy << " deg\n"
                   << "Rotation Speed: " << rotationSpeed_ << " deg/frame";

  std::string cameraInfo = cameraInfoStream.str();
  overlay_.render(cameraInfo, static_cast<int>(currentRenderMode_),
                  static_cast<int>(RenderMode::COUNT), model);

  glfwSetMouseButtonCallback(window_, Renderer::mouseButtonCallback);
}

void Renderer::drawAllFaces(const OBJModel &model) {
  MeshRenderer::drawAllFaces(model, currentRenderMode_, textureID_,
                             faceGrayColors_, faceRandomColors_,
                             faceMaterialColors_);
}

void Renderer::computeModelCenter(const OBJModel &model) {
  if (model.vertices.empty()) {
    return;
  }
  float cx, cy, cz;
  ModelUtilities::computeModelCenter(model, cx, cy, cz);
  modelTranslation_.setIdentity();
  modelTranslation_.m[12] = -cx;
  modelTranslation_.m[13] = -cy;
  modelTranslation_.m[14] = -cz;
}

void Renderer::buildFaceBasedColors(const OBJModel &model) {
  ModelUtilities::buildFaceBasedColors(model, faceGrayColors_,
                                       faceRandomColors_, faceMaterialColors_);
}

void Renderer::scrollCallback(GLFWwindow *window, double xoffset,
                              double yoffset) {
  auto *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onScroll(xoffset, yoffset);
  }
}

void Renderer::onScroll(double /*xoffset*/, double yoffset) {
  camera_.fovy -= static_cast<float>(yoffset);
  if (camera_.fovy < 1.0f)
    camera_.fovy = 1.0f;
  if (camera_.fovy > 150.0f)
    camera_.fovy = 150.0f;
}

void Renderer::keyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  auto *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onKey(key, scancode, action, mods);
  }
}

void Renderer::onKey(int key, int /*scancode*/, int action, int /*mods*/) {
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (key) {
    case GLFW_KEY_F:
      isFreeCameraMode_ = !isFreeCameraMode_;
      if (!isFreeCameraMode_) {
        resetToDefaults();
      }
      std::cout << (isFreeCameraMode_ ? "Switched to Free Camera Mode.\n"
                                      : "Switched to Focus Mode.\n");
      break;

    case GLFW_KEY_KP_ADD:
    case GLFW_KEY_EQUAL:
      if (!isFreeCameraMode_)
        rotationSpeed_ += 0.1f;
      break;
    case GLFW_KEY_KP_SUBTRACT:
    case GLFW_KEY_MINUS:
      if (!isFreeCameraMode_) {
        rotationSpeed_ = std::max(0.0f, rotationSpeed_ - 0.1f);
      }
      break;

    case GLFW_KEY_SPACE:
      if (!isFreeCameraMode_)
        resetToDefaults();
      break;

    case GLFW_KEY_T:
      if (!transitioning_) {
        int next = static_cast<int>(currentRenderMode_) + 1;
        if (next >= static_cast<int>(RenderMode::COUNT)) {
          next = 0;
        }
        nextRenderMode_ = static_cast<RenderMode>(next);
        transitioning_ = true;
        fadeOut_ = true;
        transitionElapsed_ = 0.0f;
        transitionAlpha_ = 0.0f;
      }
      break;

    case GLFW_KEY_W:
      if (isFreeCameraMode_)
        moveForward_ = true;
      break;
    case GLFW_KEY_S:
      if (isFreeCameraMode_)
        moveBackward_ = true;
      break;
    case GLFW_KEY_A:
      if (isFreeCameraMode_)
        moveLeft_ = true;
      break;
    case GLFW_KEY_D:
      if (isFreeCameraMode_)
        moveRight_ = true;
      break;
    case GLFW_KEY_Q:
      if (isFreeCameraMode_)
        moveDown_ = true;
      break;
    case GLFW_KEY_E:
      if (isFreeCameraMode_)
        moveUp_ = true;
      break;
    case GLFW_KEY_LEFT:
      if (isFreeCameraMode_)
        yawDelta_ = -1.0f;
      break;
    case GLFW_KEY_RIGHT:
      if (isFreeCameraMode_)
        yawDelta_ = 1.0f;
      break;
    case GLFW_KEY_UP:
      if (isFreeCameraMode_)
        pitchDelta_ = 1.0f;
      break;
    case GLFW_KEY_DOWN:
      if (isFreeCameraMode_)
        pitchDelta_ = -1.0f;
      break;
    default:
      break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
    case GLFW_KEY_W:
      if (isFreeCameraMode_)
        moveForward_ = false;
      break;
    case GLFW_KEY_S:
      if (isFreeCameraMode_)
        moveBackward_ = false;
      break;
    case GLFW_KEY_A:
      if (isFreeCameraMode_)
        moveLeft_ = false;
      break;
    case GLFW_KEY_D:
      if (isFreeCameraMode_)
        moveRight_ = false;
      break;
    case GLFW_KEY_Q:
      if (isFreeCameraMode_)
        moveDown_ = false;
      break;
    case GLFW_KEY_E:
      if (isFreeCameraMode_)
        moveUp_ = false;
      break;
    case GLFW_KEY_LEFT:
    case GLFW_KEY_RIGHT:
      if (isFreeCameraMode_)
        yawDelta_ = 0.0f;
      break;
    case GLFW_KEY_UP:
    case GLFW_KEY_DOWN:
      if (isFreeCameraMode_)
        pitchDelta_ = 0.0f;
      break;
    default:
      break;
    }
  }
}

void Renderer::resetToDefaults() {
  // Reset camera parameters
  camera_.eye = defaultEye_;
  camera_.center = defaultCenter_;
  camera_.up = defaultUp_;
  camera_.fovy = defaultFovy_;
  rotationAngle_ = 0.0f;
  rotationSpeed_ = defaultRotationSpeed_;
  yawDelta_ = 0.0f;
  pitchDelta_ = 0.0f;
}

void Renderer::dropCallback(GLFWwindow *window, int count, const char **paths) {
  auto *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onDrop(count, paths);
  }
}

void Renderer::onDrop(int count, const char **paths) {
  if (count <= 0)
    return;
  std::string droppedFile = paths[0];
  std::cout << "Dropped file: " << droppedFile << std::endl;

  if (droppedFile.find(".bmp") != std::string::npos) {
    loadTextureFromFile(droppedFile);
    currentModel_.textureName = droppedFile;
  } else if (droppedFile.find(".obj") != std::string::npos) {
    loadModelFromFile(droppedFile);
    currentModel_.objectName = droppedFile;
  }
}

void Renderer::loadTextureFromFile(const std::string &filePath) {
  std::cout << "Attempting to load texture: " << filePath << std::endl;
  GLuint newTexture = TextureManager::loadBMPTexture(filePath);

  if (newTexture == 0) {
    std::cerr << "Failed to load texture. Generating white fallback texture.\n";
    newTexture = TextureManager::generateWhiteTexture(64, 64);
  }

  if (textureID_ != 0) {
    glDeleteTextures(1, &textureID_);
  }
  textureID_ = newTexture;

  std::cout << "Texture loaded successfully. Texture ID: " << textureID_
            << std::endl;
}

void Renderer::loadModelFromFile(const std::string &filePath) {
  std::cout << "Attempting to load model: " << filePath << std::endl;
  OBJModel newModel;
  bool isValid = OBJLoader::loadOBJ(filePath, newModel);
  if (isValid) {
    std::cout << "Model loaded successfully.\n";
    computeModelCenter(newModel);
    buildFaceBasedColors(newModel);
    newModel.objectName = filePath;
    newModel.textureName = currentModel_.textureName;
    currentModel_ = newModel;
  } else {
    std::cerr << "Failed to load model.\n";
  }
}

void Renderer::handleFreeCameraMovement(float deltaTime) {
  float speed = 5.0f * deltaTime;
  if (moveForward_)
    camera_.moveForward(speed);
  if (moveBackward_)
    camera_.moveForward(-speed);
  if (moveRight_)
    camera_.moveRight(speed);
  if (moveLeft_)
    camera_.moveRight(-speed);
  if (moveUp_)
    camera_.moveUp(speed);
  if (moveDown_)
    camera_.moveUp(-speed);
}

void Renderer::handleFreeCameraRotation(float deltaTime) {
  float rotationSpeed = 90.0f * deltaTime;
  camera_.rotate(yawDelta_ * rotationSpeed, pitchDelta_ * rotationSpeed);
  yawDelta_ = 0.0f;
  pitchDelta_ = 0.0f;
}

void Renderer::mouseButtonCallback(GLFWwindow *window, int button, int action,
                                   int mods) {
  auto *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onMouseButton(button, action, mods);
  }
}

void Renderer::onMouseButton(int button, int action, int mods) {
  static_cast<void>(mods);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double rawMouseX, rawMouseY;
    glfwGetCursorPos(window_, &rawMouseX, &rawMouseY);

    float mouseX = static_cast<float>(rawMouseX);
    float clickY = static_cast<float>(height_) - static_cast<float>(rawMouseY);

    float openW = 200.0f;
    float openH = 100.0f;
    float openX = static_cast<float>(width_) - openW - 10.0f;
    float openY = 50.0f;

    if (mouseX >= openX && mouseX <= (openX + openW) && clickY >= openY &&
        clickY <= (openY + openH)) {
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
