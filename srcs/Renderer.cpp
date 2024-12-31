#include "Renderer.hpp"

#include <iostream>

#include <cfloat>
#include <cmath>
#include <stdexcept>

Renderer::Renderer(GLFWwindow *window, int width, int height)
    : m_window(window), m_width(width), m_height(height), m_rotationAngle(0.0f),
      m_rotationSpeed(0.5f) // for instance, 0.5 deg/frame
{
  if (!m_window) {
    throw std::runtime_error("Renderer received a null GLFWwindow*!");
  }
  glfwMakeContextCurrent(m_window);
  initializeGL();

  // Setup default camera
  m_camera.eye = {0.0f, 0.0f, 5.0f};
  m_camera.center = {0.0f, 0.0f, 0.0f};
  m_camera.up = {0.0f, 1.0f, 0.0f};

  m_camera.fovy = 45.0f;
  m_camera.aspectRatio =
      static_cast<float>(m_width) / static_cast<float>(m_height);
  m_camera.nearZ = 0.1f;
  m_camera.farZ = 100.0f;

  m_modelTranslation.setIdentity();

  // Store defaults so we can restore them on Spacebar
  m_defaultEye = m_camera.eye;
  m_defaultCenter = m_camera.center;
  m_defaultUp = m_camera.up;
  m_defaultFovy = m_camera.fovy;
  m_defaultRotationSpeed = m_rotationSpeed;

  // Set up scroll callback (zoom) and key callback (movement, reset)
  glfwSetWindowUserPointer(m_window, this);
  glfwSetScrollCallback(m_window, Renderer::scrollCallback);
  glfwSetKeyCallback(m_window, Renderer::keyCallback);
}

Renderer::~Renderer() {
  // Window destruction is handled by main()
}

void Renderer::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glViewport(0, 0, m_width, m_height);
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
}

void Renderer::run(const OBJModel &model) {
  computeModelCenter(model);

  while (!glfwWindowShouldClose(m_window)) {
    renderFrame(model);
    glfwSwapBuffers(m_window);
    glfwPollEvents();
  }
}

void Renderer::renderFrame(const OBJModel &model) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 1) Projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Matrix4 proj = m_camera.getProjectionMatrix();
  glLoadMatrixf(proj.m);

  // 2) ModelView
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  Matrix4 view = m_camera.getViewMatrix();

  // Rotate around Y using the speed factor
  m_rotationAngle += m_rotationSpeed; // degrees per frame
  float radians = m_rotationAngle * 3.1415926535f / 180.0f;

  Matrix4 rotation;
  rotation.setIdentity();
  rotation.m[0] = cosf(radians);
  rotation.m[2] = sinf(radians);
  rotation.m[8] = -sinf(radians);
  rotation.m[10] = cosf(radians);

  // modelMatrix = rotation * translation
  Matrix4 modelMatrix = Matrix4::multiply(rotation, m_modelTranslation);
  Matrix4 modelView = Matrix4::multiply(view, modelMatrix);
  glLoadMatrixf(modelView.m);

  // 3) Draw
  for (const auto &face : model.faces) {
    glBegin(GL_POLYGON);
    for (const auto &fv : face.vertices) {
      const auto &v = model.vertices[fv.vertexIndex];
      glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
  }
  renderOverlay();
}

void Renderer::computeModelCenter(const OBJModel &model) {
  if (model.vertices.empty()) {
    return;
  }

  float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
  float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

  for (const auto &v : model.vertices) {
    if (v.x < minX)
      minX = v.x;
    if (v.y < minY)
      minY = v.y;
    if (v.z < minZ)
      minZ = v.z;
    if (v.x > maxX)
      maxX = v.x;
    if (v.y > maxY)
      maxY = v.y;
    if (v.z > maxZ)
      maxZ = v.z;
  }

  float centerX = 0.5f * (minX + maxX);
  float centerY = 0.5f * (minY + maxY);
  float centerZ = 0.5f * (minZ + maxZ);

  m_modelTranslation.setIdentity();
  m_modelTranslation.m[12] = -centerX;
  m_modelTranslation.m[13] = -centerY;
  m_modelTranslation.m[14] = -centerZ;
}

void Renderer::scrollCallback(GLFWwindow *window, double xoffset,
                              double yoffset) {
  Renderer *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onScroll(xoffset, yoffset);
  }
}

void Renderer::onScroll(double /*xoffset*/, double yoffset) {
  // Zoom by changing the FOV
  // negative yoffset => zoom out, positive => zoom in
  m_camera.fovy -= static_cast<float>(yoffset);

  // Clamp the FOV
  if (m_camera.fovy < 1.0f)
    m_camera.fovy = 1.0f;
  if (m_camera.fovy > 120.0f)
    m_camera.fovy = 120.0f;

  // Debugging output
  // std::cout << "New FOV: " << m_camera.fovy << "\n";
}

void Renderer::keyCallback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
  // Retrieve the 'Renderer*' from user pointer
  auto *renderer =
      reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
  if (renderer) {
    renderer->onKey(key, scancode, action, mods);
  }
}

void Renderer::onKey(int key, int /*scancode*/, int action, int /*mods*/) {
  // We only act on PRESS or REPEAT
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (key) {
    // --- Move camera "eye" up / down / left / right
    case GLFW_KEY_UP:
      m_camera.eye.y += 0.1f; // Move up by 0.1
      break;
    case GLFW_KEY_DOWN:
      m_camera.eye.y -= 0.1f; // Move down
      break;
    case GLFW_KEY_LEFT:
      m_camera.eye.x -= 0.1f; // Move left
      break;
    case GLFW_KEY_RIGHT:
      m_camera.eye.x += 0.1f; // Move right
      break;

    // --- Zoom forward/back (alternative approach)
    // If you prefer to move the camera on Z instead of adjusting fovy:
    case GLFW_KEY_W:
      m_camera.eye.z -= 0.1f; // move closer
      break;
    case GLFW_KEY_S:
      m_camera.eye.z += 0.1f; // move farther
      break;

    // --- Speed up / slow down rotation
    case GLFW_KEY_KP_ADD: // or '+'
    case GLFW_KEY_EQUAL:  // typical US keyboard for '+'
      m_rotationSpeed += 0.1f;
      break;
    case GLFW_KEY_KP_SUBTRACT: // or '-'
    case GLFW_KEY_MINUS:
      m_rotationSpeed -= 0.1f;
      if (m_rotationSpeed < 0.0f) {
        m_rotationSpeed = 0.0f;
      }
      break;

    // --- Reset (spacebar)
    case GLFW_KEY_SPACE:
      resetToDefaults();
      break;

    default:
      break;
    }
  }
}

void Renderer::resetToDefaults() {
  m_camera.eye = m_defaultEye;
  m_camera.center = m_defaultCenter;
  m_camera.up = m_defaultUp;
  m_camera.fovy = m_defaultFovy;

  m_rotationAngle = 0.0f;
  m_rotationSpeed = m_defaultRotationSpeed;
}

void Renderer::drawText(float x, float y, const char *text) {
  // Position text in 2D screen-space:
  glRasterPos2f(x, y);

  // Draw each character:
  for (const char *p = text; *p != '\0'; p++) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *p);
  }
}

void Renderer::renderOverlay() {
  // 1) Save current matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  // 2) Set up an orthographic projection matching the window size
  //    e.g., (0,0) is bottom-left, (m_width, m_height) is top-right.
  //    If you want top-left to be (0,0), just flip the y-axis.
  glOrtho(0, m_width, 0, m_height, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Disable depth test so text is always visible
  glDisable(GL_DEPTH_TEST);

  // We'll draw some lines of text, spacing them vertically
  float lineHeight = 18.0f; // spacing between text lines
  float xPos = 10.0f;
  float yPos = m_height - lineHeight - 100; // start from top

  // 1) Show camera info
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Camera Eye: (%.2f, %.2f, %.2f)",
           m_camera.eye.x, m_camera.eye.y, m_camera.eye.z);
  drawText(xPos, yPos, buffer);
  yPos -= lineHeight;

  snprintf(buffer, sizeof(buffer), "Camera Center: (%.2f, %.2f, %.2f)",
           m_camera.center.x, m_camera.center.y, m_camera.center.z);
  drawText(xPos, yPos, buffer);
  yPos -= lineHeight;

  snprintf(buffer, sizeof(buffer), "Camera Up: (%.2f, %.2f, %.2f)",
           m_camera.up.x, m_camera.up.y, m_camera.up.z);
  drawText(xPos, yPos, buffer);
  yPos -= lineHeight;

  snprintf(buffer, sizeof(buffer), "FOV: %.2f deg", m_camera.fovy);
  drawText(xPos, yPos, buffer);
  yPos -= lineHeight;

  // 2) Rotation info
  snprintf(buffer, sizeof(buffer), "Rotation Speed: %.2f deg/frame",
           m_rotationSpeed);
  drawText(xPos, yPos, buffer);
  yPos -= lineHeight;

  // 3) Controls
  drawText(xPos, yPos, "Controls:");
  yPos -= lineHeight;
  drawText(xPos + 20.0f, yPos, "Arrow Keys: Move Camera (x,y)");
  yPos -= lineHeight;
  drawText(xPos + 20.0f, yPos, "W / S: Move Camera Forward / Back");
  yPos -= lineHeight;
  drawText(xPos + 20.0f, yPos, "+ / -: Increase / Decrease Rotation Speed");
  yPos -= lineHeight;
  drawText(xPos + 20.0f, yPos, "Spacebar: Reset to default camera + speed");
  yPos -= lineHeight;

  // Re-enable depth
  glEnable(GL_DEPTH_TEST);

  // Restore matrices
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}