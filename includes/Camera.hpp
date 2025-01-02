#pragma once

#include "Matrix4.hpp"
#include "Vector3.hpp"

/**
 * @brief Represents a camera in 3D space.
 */
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

  Camera();
  Matrix4 getViewMatrix() const;
  Matrix4 getProjectionMatrix() const;
};
