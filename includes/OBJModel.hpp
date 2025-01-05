#pragma once

#include <string>
#include <vector>

enum class RenderMode {
  GRAYSCALE = 0,
  RANDOM_COLOR,
  WIRE_FRAME,
  TEXTURE,
  COUNT
};

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
  std::string objectName;
  std::string textureName;
  std::vector<Vertex> vertices;
  std::vector<TexCoord> texCoords;
  std::vector<Normal> normals;
  std::vector<Face> faces;
};
