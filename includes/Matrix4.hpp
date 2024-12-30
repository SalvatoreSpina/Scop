#pragma once

#include <cmath>   // for tanf
#include <cstring> // for memset

#include "Vector3.hpp"

struct Matrix4 {
  // Stored in column-major order to be directly usable by OpenGL
  float m[16];

  Matrix4() { setIdentity(); }

  void setIdentity() {
    // clang-format off
        m[0] = 1;  m[1] = 0;  m[2] = 0;  m[3] = 0;
        m[4] = 0;  m[5] = 1;  m[6] = 0;  m[7] = 0;
        m[8] = 0;  m[9] = 0;  m[10] = 1; m[11] = 0;
        m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    // clang-format on
  }

  // Multiply two Matrix4 (this * rhs), result stored in 'out'.
  static Matrix4 multiply(const Matrix4 &lhs, const Matrix4 &rhs) {
    Matrix4 out;
    for (int row = 0; row < 4; ++row) {
      for (int col = 0; col < 4; ++col) {
        float sum = 0.0f;
        for (int k = 0; k < 4; ++k) {
          sum += lhs.m[row + k * 4] * rhs.m[k + col * 4];
        }
        out.m[row + col * 4] = sum;
      }
    }
    return out;
  }

  /// Creates a "look-at" view matrix, from an eye position,
  /// looking at a center position, with a given up vector.
  static Matrix4 lookAt(const Vector3 &eye, const Vector3 &center,
                        const Vector3 &up) {
    Vector3 f = (center - eye).normalize(); // forward
    Vector3 s = f.cross(up).normalize();    // right
    Vector3 u = s.cross(f);                 // true up

    Matrix4 view;
    // clang-format off
        view.m[0] = s.x;   view.m[4] = s.y;   view.m[8]  = s.z;   view.m[12] = -eye.dot(s);
        view.m[1] = u.x;   view.m[5] = u.y;   view.m[9]  = u.z;   view.m[13] = -eye.dot(u);
        view.m[2] = -f.x;  view.m[6] = -f.y;  view.m[10] = -f.z;  view.m[14] =  eye.dot(f);
        view.m[3] = 0;     view.m[7] = 0;     view.m[11] = 0;     view.m[15] = 1;
    // clang-format on

    return view;
  }

  /// Creates a perspective projection matrix given:
  ///   fovy (vertical field of view in degrees)
  ///   aspect (width / height ratio)
  ///   nearZ (near clipping plane)
  ///   farZ (far clipping plane)
  static Matrix4 perspective(float fovy, float aspect, float nearZ,
                             float farZ) {
    Matrix4 proj;
    proj.setIdentity();

    float fovyRad = fovy * 3.1415926535f / 180.0f;
    float f = 1.0f / std::tan(fovyRad / 2.0f);

    proj.m[0] = f / aspect; // scale the x coordinates
    proj.m[5] = f;          // scale the y coordinates
    proj.m[10] = (farZ + nearZ) / (nearZ - farZ);
    proj.m[11] = -1.0f;
    proj.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    proj.m[15] = 0.0f;

    return proj;
  }
};
