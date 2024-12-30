#include "Window.hpp"

#include <iostream>

bool WindowManager::initializeGLFW() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW.\n";
    return false;
  }
  return true;
}

GLFWwindow *WindowManager::createWindow(Window windowConfig) {
  int width = windowConfig.width;
  int height = windowConfig.height;
  const char *title = windowConfig.title;
  GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window.\n";
    glfwTerminate();
  }
  return window;
}

void WindowManager::mainLoop(GLFWwindow *window, const OBJModel &model) {
  Camera camera;
  camera.eye = Vector3(0.0f, 0.0f, 5.0f);
  camera.center = Vector3(0.0f, 0.0f, 0.0f);
  camera.up = Vector3(0.0f, 1.0f, 0.0f);
  camera.fovy = 45.0f;
  camera.aspectRatio = 800.0f / 600.0f;
  camera.nearZ = 0.1f;
  camera.farZ = 100.0f;

  float rotationAngle = 0.0f;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(camera.getProjectionMatrix().m);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(camera.getViewMatrix().m);

    rotationAngle += 0.5f;
    Matrix4 modelRotation;
    float radians = rotationAngle * 3.1415926535f / 180.0f;
    modelRotation.setIdentity();
    modelRotation.m[0] = cosf(radians);
    modelRotation.m[2] = sinf(radians);
    modelRotation.m[8] = -sinf(radians);
    modelRotation.m[10] = cosf(radians);

    Matrix4 finalModelView =
        Matrix4::multiply(camera.getViewMatrix(), modelRotation);
    glLoadMatrixf(finalModelView.m);

    for (const Face &face : model.faces) {
      glBegin(GL_POLYGON);
      for (const FaceVertex &vertex : face.vertices) {
        const Vertex &v = model.vertices[vertex.vertexIndex];
        glVertex3f(v.x, v.y, v.z);
      }
      glEnd();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
