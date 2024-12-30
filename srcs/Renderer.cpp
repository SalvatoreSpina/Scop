#include "Renderer.hpp"

#include <iostream>

#include <cfloat>
#include <cmath>
#include <stdexcept>

Renderer::Renderer(GLFWwindow *window, int width, int height)
    : m_window(window), m_width(width), m_height(height),
      m_rotationAngle(0.0f) {
  if (!m_window) {
    throw std::runtime_error("Renderer received a null GLFWwindow*!");
  }
  glfwMakeContextCurrent(m_window);
  initializeGL();

  // Default camera
  m_camera.eye = {0.0f, 0.0f, 5.0f};
  m_camera.center = {0.0f, 0.0f, 0.0f};
  m_camera.up = {0.0f, 1.0f, 0.0f};

  // Projection
  m_camera.fovy = 45.0f;
  m_camera.aspectRatio =
      static_cast<float>(m_width) / static_cast<float>(m_height);
  m_camera.nearZ = 0.1f;
  m_camera.farZ = 100.0f;

  // Identity translation until computed
  m_modelTranslation.setIdentity();

  // Set up scroll callback for zoom
  glfwSetWindowUserPointer(m_window, this);
  glfwSetScrollCallback(m_window, Renderer::scrollCallback);
}

Renderer::~Renderer() {
  // Cleanup window is done by the caller
}

void Renderer::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glViewport(0, 0, m_width, m_height);
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
}

void Renderer::run(const OBJModel &model) {
  // Center the model once
  std::cout << "About to compute center. model.vertices.size() = "
            << model.vertices.size() << std::endl;
  std::cout << "First vertex: " << model.vertices[0].x << ", "
            << model.vertices[0].y << ", " << model.vertices[0].z << std::endl;

  computeModelCenter(model); // <-- Crash?

  // Main loop
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
  auto proj = m_camera.getProjectionMatrix();
  glLoadMatrixf(proj.m);

  // 2) ModelView
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  auto view = m_camera.getViewMatrix();

  // Rotate around Y
  m_rotationAngle += 0.5f;
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
  for (const Face &face : model.faces) {
    glBegin(GL_POLYGON);
    for (const FaceVertex &fv : face.vertices) {
      const Vertex &v = model.vertices[fv.vertexIndex];
      glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
  }
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