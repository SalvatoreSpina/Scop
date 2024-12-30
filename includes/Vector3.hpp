#pragma once

#include <cmath>

struct Vector3 {
  float x, y, z;

  Vector3() : x(0), y(0), z(0) {}
  Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

  Vector3 operator+(const Vector3 &rhs) const {
    return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  Vector3 operator-(const Vector3 &rhs) const {
    return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  Vector3 operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
  }

  float dot(const Vector3 &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  Vector3 cross(const Vector3 &rhs) const {
    return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                   x * rhs.y - y * rhs.x);
  }

  float length() const { return std::sqrt(x * x + y * y + z * z); }

  Vector3 normalize() const {
    float len = length();
    return (len > 1e-5f) ? Vector3(x / len, y / len, z / len) : Vector3();
  }
};
