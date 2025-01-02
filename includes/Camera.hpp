#pragma once

#include "Matrix4.hpp"
#include "Vector3.hpp"

/**
 * @brief Represents a camera in 3D space with support for both Focus and Free
 * Camera Modes.
 */
class Camera {
public:
  // Camera Attributes
  Vector3 eye;    ///< Position of the camera.
  Vector3 center; ///< Point the camera is looking at (used in Focus Mode).
  Vector3 up;     ///< Up direction for the camera.

  // Camera Parameters
  float fovy;        ///< Field of view in the y direction (in degrees).
  float aspectRatio; ///< Aspect ratio of the view.
  float nearZ;       ///< Near clipping plane.
  float farZ;        ///< Far clipping plane.

  // Direction Vectors (used in Free Camera Mode)
  Vector3 forward; ///< Forward direction vector.
  Vector3 right;   ///< Right direction vector.

  /**
   * @brief Constructs a Camera with default parameters.
   */
  Camera();

  /**
   * @brief Returns the view matrix based on current camera parameters.
   *        If freeMode is true, constructs the view matrix for Free Camera
   * Mode. Otherwise, constructs for Focus Mode.
   * @param freeMode Determines which mode to use.
   * @return View matrix.
   */
  Matrix4 getViewMatrix(bool freeMode = false) const;

  /**
   * @brief Returns the projection matrix based on current camera parameters.
   * @return Projection matrix.
   */
  Matrix4 getProjectionMatrix() const;

  /**
   * @brief Moves the camera forward or backward in Free Camera Mode.
   * @param distance Distance to move. Positive values move forward; negative
   * move backward.
   */
  void moveForward(float distance);

  /**
   * @brief Moves the camera right or left in Free Camera Mode.
   * @param distance Distance to move. Positive values move right; negative move
   * left.
   */
  void moveRight(float distance);

  /**
   * @brief Moves the camera up or down in Free Camera Mode.
   * @param distance Distance to move. Positive values move up; negative move
   * down.
   */
  void moveUp(float distance);

  /**
   * @brief Rotates the camera based on yaw and pitch angles in Free Camera
   * Mode.
   * @param yaw Angle in degrees to rotate around the up vector.
   * @param pitch Angle in degrees to rotate around the right vector.
   */
  void rotate(float yaw, float pitch);

private:
  /**
   * @brief Updates the direction vectors based on current orientation.
   */
  void updateDirectionVectors();
};
