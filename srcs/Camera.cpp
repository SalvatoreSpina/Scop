#include "Camera.hpp"

/**
 * @brief Default constructor initializing camera parameters and direction
 * vectors.
 */
Camera::Camera()
    : eye(0, 0, 5), center(0, 0, 0), up(0, 1, 0), fovy(45.0f),
      aspectRatio(4.0f / 3.0f), nearZ(0.1f), farZ(100.0f) {
  // Initialize direction vectors for Free Camera Mode
  forward = (center - eye).normalize();
  right = forward.cross(up).normalize();
}

/**
 * @brief Constructs a view matrix using either Focus Mode or Free Camera Mode.
 * @param freeMode If true, constructs the view matrix for Free Camera Mode.
 * @return View matrix.
 */
Matrix4 Camera::getViewMatrix(bool freeMode) const {
  Matrix4 view;

  if (!freeMode) {
    // Focus Mode: Look at the center point
    Vector3 f = (center - eye).normalize();
    Vector3 s = f.cross(up).normalize();
    Vector3 u = s.cross(f);

    view.setIdentity();
    view.m[0] = s.x;
    view.m[1] = u.x;
    view.m[2] = -f.x;
    view.m[4] = s.y;
    view.m[5] = u.y;
    view.m[6] = -f.y;
    view.m[8] = s.z;
    view.m[9] = u.z;
    view.m[10] = -f.z;
    view.m[12] = -s.dot(eye);
    view.m[13] = -u.dot(eye);
    view.m[14] = f.dot(eye);
  } else {
    // Free Camera Mode: Look in the forward direction
    Vector3 f = forward.normalize();
    Vector3 s = f.cross(up).normalize();
    Vector3 u = s.cross(f).normalize();

    view.setIdentity();
    view.m[0] = s.x;
    view.m[1] = u.x;
    view.m[2] = -f.x;
    view.m[4] = s.y;
    view.m[5] = u.y;
    view.m[6] = -f.y;
    view.m[8] = s.z;
    view.m[9] = u.z;
    view.m[10] = -f.z;
    view.m[12] = -s.dot(eye);
    view.m[13] = -u.dot(eye);
    view.m[14] = f.dot(eye);
  }

  return view;
}

/**
 * @brief Constructs the Perspective matrix
 */
Matrix4 Camera::getProjectionMatrix() const {
  return Matrix4::perspective(fovy, aspectRatio, nearZ, farZ);
}

/**
 * @brief Moves the camera forward or backward in Free Camera Mode.
 */
void Camera::moveForward(float distance) { eye += forward * distance; }

/**
 * @brief Moves the camera right or left in Free Camera Mode.
 */
void Camera::moveRight(float distance) { eye += right * distance; }

/**
 * @brief Moves the camera up or down in Free Camera Mode.
 */
void Camera::moveUp(float distance) { eye += up * distance; }

/**
 * @brief Rotates the camera based on yaw and pitch angles in Free Camera Mode.
 */
void Camera::rotate(float yaw, float pitch) {
  // Convert degrees to radians
  float yawRad = yaw * (3.14159265358979323846f / 180.0f);
  float pitchRad = pitch * (3.14159265358979323846f / 180.0f);

  // Create rotation matrices
  Matrix4 yawMatrix;
  yawMatrix.setIdentity();
  yawMatrix.m[0] = cosf(yawRad);
  yawMatrix.m[2] = sinf(yawRad);
  yawMatrix.m[8] = -sinf(yawRad);
  yawMatrix.m[10] = cosf(yawRad);

  Matrix4 pitchMatrix;
  pitchMatrix.setIdentity();
  pitchMatrix.m[5] = cosf(pitchRad);
  pitchMatrix.m[6] = -sinf(pitchRad);
  pitchMatrix.m[9] = sinf(pitchRad);
  pitchMatrix.m[10] = cosf(pitchRad);

  // Apply rotations to the forward vector
  Matrix4 combinedRotation = Matrix4::multiply(yawMatrix, pitchMatrix);
  forward = combinedRotation.transform(forward).normalize();

  // Recalculate the right vector
  right = forward.cross(up).normalize();
}
