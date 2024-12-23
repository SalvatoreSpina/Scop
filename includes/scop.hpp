#pragma once

#include <vector>
#include <string>

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

bool loadOBJ(const std::string& filePath, OBJModel& model);