#include "scop.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

bool loadOBJ(const std::string &filePath, OBJModel &model) {
  std::ifstream inFile(filePath);
  if (!inFile) {
    std::cerr << "Cannot open the .obj file: " << filePath << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(inFile, line)) {
    // Trim leading/trailing whitespace by creating a stringstream
    // and extracting tokens carefully.
    std::istringstream ss(line);

    std::string prefix;
    ss >> prefix;

    // Skip empty lines or commented lines
    if (prefix.empty() || prefix[0] == '#') {
      continue;
    }

    if (prefix == "v") {
      // Vertex position
      Vertex vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;
      model.vertices.push_back(vertex);
    } else if (prefix == "vt") {
      // Texture coordinate
      TexCoord texCoord;
      ss >> texCoord.u >> texCoord.v;
      // 'w' is optional in OBJ
      if (!(ss >> texCoord.w)) {
        texCoord.w = 0.0f;
      }
      model.texCoords.push_back(texCoord);
    } else if (prefix == "vn") {
      // Vertex normal
      Normal normal;
      ss >> normal.x >> normal.y >> normal.z;
      model.normals.push_back(normal);
    } else if (prefix == "f") {
      // Face data
      Face face;
      std::string vertexStr;
      while (ss >> vertexStr) {
        // Each face vertex can be: v, v/t, v//n, or v/t/n.
        FaceVertex fv = {-1, -1, -1};

        // Locate the first and second slash
        size_t pos1 = vertexStr.find('/');
        if (pos1 == std::string::npos) {
          // Format: "v"
          // Only the vertex index is provided.
          fv.vertexIndex = std::stoi(vertexStr) - 1;
        } else {
          // There's at least one slash. Grab the vertex index first:
          fv.vertexIndex = std::stoi(vertexStr.substr(0, pos1)) - 1;

          // Now find the second slash, if any:
          size_t pos2 = vertexStr.find('/', pos1 + 1);

          if (pos2 == std::string::npos) {
            // Format: "v/t"
            // Only texture coordinate is after the first slash:
            std::string texPart = vertexStr.substr(pos1 + 1);
            if (!texPart.empty()) {
              fv.texCoordIndex = std::stoi(texPart) - 1;
            }
          } else {
            // There's a second slash. Could be "v//n" or "v/t/n".
            if (pos2 == pos1 + 1) {
              // Format: "v//n" => no texture coordinate
              // Normal index is after the second slash:
              std::string normPart = vertexStr.substr(pos2 + 1);
              if (!normPart.empty()) {
                fv.normalIndex = std::stoi(normPart) - 1;
              }
            } else {
              // Format: "v/t/n"
              // Extract t from pos1+1 .. pos2-1
              std::string texPart =
                  vertexStr.substr(pos1 + 1, pos2 - (pos1 + 1));
              if (!texPart.empty()) {
                fv.texCoordIndex = std::stoi(texPart) - 1;
              }

              // Normal index is everything after pos2+1
              std::string normPart = vertexStr.substr(pos2 + 1);
              if (!normPart.empty()) {
                fv.normalIndex = std::stoi(normPart) - 1;
              }
            }
          }
        }

        face.vertices.push_back(fv);
      }
      model.faces.push_back(face);
    }
  }

  inFile.close();
  return true;
}
