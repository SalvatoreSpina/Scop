#include "Renderer.hpp"

#include <cmath>
#include <stdexcept>

Renderer::Renderer(GLFWwindow *window, int width, int height)
    : m_window(window), m_width(width), m_height(height),
      m_rotationAngle(0.0f) {
  if (!m_window) {
    throw std::runtime_error("Renderer received a null GLFWwindow*!");
  }
  // Make sure this context is current
  glfwMakeContextCurrent(m_window);

  // Initialize any OpenGL state
  initializeGL();

  // Setup default camera
  m_camera.eye = {0.0f, 0.0f, 5.0f};
  m_camera.center = {0.0f, 0.0f, 0.0f};
  m_camera.up = {0.0f, 1.0f, 0.0f};

  // Projection params
  m_camera.fovy = 45.0f;
  m_camera.aspectRatio =
      static_cast<float>(m_width) / static_cast<float>(m_height);
  m_camera.nearZ = 0.1f;
  m_camera.farZ = 100.0f;
}

Renderer::~Renderer() {
  // We do not destroy the window here
  // The caller (main) is responsible for that
}

void Renderer::initializeGL() {
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Set viewport
  glViewport(0, 0, m_width, m_height);

  // Optional: set a background color
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
}

void Renderer::run(const OBJModel &model) {
  // Main loop
  while (!glfwWindowShouldClose(m_window)) {
    renderFrame(model);

    // Swap buffers
    glfwSwapBuffers(m_window);

    // Poll events
    glfwPollEvents();
  }
}

void Renderer::renderFrame(const OBJModel &model) {
  // Clear color and depth
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // -----------------
  // 1) Projection
  // -----------------
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  auto proj = m_camera.getProjectionMatrix();
  glLoadMatrixf(proj.m);

  // -----------------
  // 2) ModelView
  // -----------------
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  auto view = m_camera.getViewMatrix();

  // Simple rotation around Y-axis
  m_rotationAngle += 0.5f; // degrees per frame
  float radians = m_rotationAngle * 3.1415926535f / 180.0f;

  // Build a rotation matrix manually
  Matrix4 rotation;
  rotation.setIdentity();
  rotation.m[0] = std::cos(radians);
  rotation.m[2] = std::sin(radians);
  rotation.m[8] = -std::sin(radians);
  rotation.m[10] = std::cos(radians);

  // Combine view and rotation => final modelview
  Matrix4 modelView = Matrix4::multiply(view, rotation);
  glLoadMatrixf(modelView.m);

  // -----------------
  // 3) Draw the model
  // -----------------
  for (const Face &face : model.faces) {
    // We'll assume polygon faces
    glBegin(GL_POLYGON);
    for (const FaceVertex &fv : face.vertices) {
      const Vertex &v = model.vertices[fv.vertexIndex];

      // OPTIONAL: set color here if you want, e.g. per-face or per-vertex
      // glColor3f(...);

      glVertex3f(v.x, v.y, v.z);
    }
    glEnd();
  }
}
