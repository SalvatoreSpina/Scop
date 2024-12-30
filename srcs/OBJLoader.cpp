#include "OBJLoader.hpp"

FaceVertex OBJLoader::parseFaceVertex(const std::string& vertexStr) {
  FaceVertex fv = {-1, -1, -1};

  size_t pos1 = vertexStr.find('/');
  if (pos1 == std::string::npos) {
    fv.vertexIndex = std::stoi(vertexStr) - 1;
    return fv;
  }

  fv.vertexIndex = std::stoi(vertexStr.substr(0, pos1)) - 1;
  size_t pos2 = vertexStr.find('/', pos1 + 1);

  if (pos2 == std::string::npos) {
    std::string texPart = vertexStr.substr(pos1 + 1);
    if (!texPart.empty()) {
      fv.texCoordIndex = std::stoi(texPart) - 1;
    }
  } else {
    if (pos2 == pos1 + 1) {
      std::string normPart = vertexStr.substr(pos2 + 1);
      if (!normPart.empty()) {
        fv.normalIndex = std::stoi(normPart) - 1;
      }
    } else {
      std::string texPart = vertexStr.substr(pos1 + 1, pos2 - (pos1 + 1));
      if (!texPart.empty()) {
        fv.texCoordIndex = std::stoi(texPart) - 1;
      }
      std::string normPart = vertexStr.substr(pos2 + 1);
      if (!normPart.empty()) {
        fv.normalIndex = std::stoi(normPart) - 1;
      }
    }
  }

  return fv;
}

void OBJLoader::parseVertex(std::istringstream& ss, OBJModel& model) {
  Vertex vertex;
  ss >> vertex.x >> vertex.y >> vertex.z;
  model.vertices.push_back(vertex);
}

void OBJLoader::parseTexCoord(std::istringstream& ss, OBJModel& model) {
  TexCoord texCoord;
  ss >> texCoord.u >> texCoord.v;
  if (!(ss >> texCoord.w)) {
    texCoord.w = 0.0f;
  }
  model.texCoords.push_back(texCoord);
}

void OBJLoader::parseNormal(std::istringstream& ss, OBJModel& model) {
  Normal normal;
  ss >> normal.x >> normal.y >> normal.z;
  model.normals.push_back(normal);
}

void OBJLoader::parseFace(std::istringstream& ss, OBJModel& model) {
  Face face;
  std::string vertexStr;
  while (ss >> vertexStr) {
    face.vertices.push_back(parseFaceVertex(vertexStr));
  }
  model.faces.push_back(face);
}

bool OBJLoader::parseLine(const std::string& line, OBJModel& model) {
  std::istringstream ss(line);
  std::string prefix;
  ss >> prefix;

  if (prefix.empty() || prefix[0] == '#') {
    return false;
  }

  if (prefix == "v") {
    parseVertex(ss, model);
  } else if (prefix == "vt") {
    parseTexCoord(ss, model);
  } else if (prefix == "vn") {
    parseNormal(ss, model);
  } else if (prefix == "f") {
    parseFace(ss, model);
  }

  return true;
}

bool OBJLoader::loadOBJ(const std::string& filePath, OBJModel& model) {
  std::ifstream inFile(filePath);
  if (!inFile) {
    std::cerr << "Cannot open the .obj file: " << filePath << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(inFile, line)) {
    parseLine(line, model);
  }

  inFile.close();
  return true;
}
