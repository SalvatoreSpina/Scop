#pragma once

#include "Matrix4.hpp"
#include "Vector3.hpp"

class Camera {
public:
  // Camera parameters
  Vector3 eye;    // The position of your camera
  Vector3 center; // Where you are looking at
  Vector3 up;     // The up direction of your camera

  // Projection parameters
  float fovy;        // Vertical Field of View, in degrees
  float aspectRatio; // aspect ratio = width / height
  float nearZ;       // near clipping plane
  float farZ;        // far clipping plane

  Camera()
      : eye(0, 0, 5), center(0, 0, 0), up(0, 1, 0), fovy(45.0f),
        aspectRatio(4.0f / 3.0f), nearZ(0.1f), farZ(100.0f) {}

  // Construct the camera’s View matrix (LookAt).
  Matrix4 getViewMatrix() const { return Matrix4::lookAt(eye, center, up); }

  // Construct the Perspective matrix
  Matrix4 getProjectionMatrix() const {
    return Matrix4::perspective(fovy, aspectRatio, nearZ, farZ);
  }
};
