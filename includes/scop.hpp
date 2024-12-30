#pragma once

#include <GL/glew.h> // Must be included before <GL/gl.h> and <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <cmath> // For tanf()
#include <iostream>
#include <string>
#include <vector>

struct Vertex {
  float x, y, z;
};

struct TexCoord {
  float u, v, w;
};

struct Normal {
  float x, y, z;
};

struct FaceVertex {
  int vertexIndex;
  int texCoordIndex;
  int normalIndex;
};

struct Face {
  std::vector<FaceVertex> vertices;
};

struct OBJModel {
  std::vector<Vertex> vertices;
  std::vector<TexCoord> texCoords;
  std::vector<Normal> normals;
  std::vector<Face> faces;
};

struct Vec3 {
  float x, y, z;

  Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

  Vec3 operator-(const Vec3 &v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
  }
  Vec3 operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }

  float dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }
  Vec3 cross(const Vec3 &v) const {
    return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }

  float magnitude() const { return std::sqrt(x * x + y * y + z * z); }

  Vec3 normalize() const {
    float mag = magnitude();
    if (mag == 0)
      return Vec3(0, 0, 0);
    return Vec3(x / mag, y / mag, z / mag);
  }
};

struct Mat4 {
  float elements[16] = {0};

  static Mat4 identity() {
    Mat4 result;
    result.elements[0] = result.elements[5] = result.elements[10] =
        result.elements[15] = 1.0f;
    return result;
  }

  static Mat4 perspective(float fov, float aspect, float near, float far) {
    Mat4 result;
    float tanHalfFOV = tanf(fov / 2.0f);
    result.elements[0] = 1.0f / (aspect * tanHalfFOV);
    result.elements[5] = 1.0f / tanHalfFOV;
    result.elements[10] = -(far + near) / (far - near);
    result.elements[11] = -1.0f;
    result.elements[14] = -(2.0f * far * near) / (far - near);
    return result;
  }

  static Mat4 lookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
    Vec3 zAxis = (eye - target).normalize();
    Vec3 xAxis = up.cross(zAxis).normalize();
    Vec3 yAxis = zAxis.cross(xAxis);

    Mat4 result = Mat4::identity();
    result.elements[0] = xAxis.x;
    result.elements[1] = yAxis.x;
    result.elements[2] = zAxis.x;

    result.elements[4] = xAxis.y;
    result.elements[5] = yAxis.y;
    result.elements[6] = zAxis.y;

    result.elements[8] = xAxis.z;
    result.elements[9] = yAxis.z;
    result.elements[10] = zAxis.z;

    result.elements[12] = -xAxis.dot(eye);
    result.elements[13] = -yAxis.dot(eye);
    result.elements[14] = -zAxis.dot(eye);

    return result;
  }
};
