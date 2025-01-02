#include "Camera.hpp"
#include <cmath>

/**
 * @brief Default constructor initializing camera parameters.
 */
Camera::Camera()
    : eye(0, 0, 5), center(0, 0, 0), up(0, 1, 0), fovy(45.0f),
      aspectRatio(4.0f / 3.0f), nearZ(0.1f), farZ(100.0f) {}

// Construct the camera’s View matrix (LookAt).
Matrix4 Camera::getViewMatrix() const {
  return Matrix4::lookAt(eye, center, up);
}

// Construct the Perspective matrix
Matrix4 Camera::getProjectionMatrix() const {
  return Matrix4::perspective(fovy, aspectRatio, nearZ, farZ);
}