#include "scop.hpp"

#include <ostream>
#include <iostream>
#include <fstream>
#include <sstream>

bool loadOBJ(const std::string& filePath, OBJModel& model) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Cannot open the .obj file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        // Trim leading/trailing whitespace
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix.empty() || prefix[0] == '#') continue; // Skip comments and empty lines

        if (prefix == "v") {
            Vertex vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            model.vertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            TexCoord texCoord;
            ss >> texCoord.u >> texCoord.v;
            // 'w' is optional
            if (!(ss >> texCoord.w)) {
                texCoord.w = 0.0f;
            }
            model.texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            Normal normal;
            ss >> normal.x >> normal.y >> normal.z;
            model.normals.push_back(normal);
        }
        else if (prefix == "f") {
            Face face;
            std::string vertexStr;
            while (ss >> vertexStr) {
                FaceVertex fv = { -1, -1, -1 };
                size_t pos1 = vertexStr.find('/');
                size_t pos2 = vertexStr.find('/', pos1 + 1);

                if (pos1 == std::string::npos) {
                    // Only vertex index
                    fv.vertexIndex = std::stoi(vertexStr) - 1;
                }
                else if (pos2 == std::string::npos || pos2 == pos1 + 1) {
                    // Vertex and texture index
                    fv.vertexIndex = std::stoi(vertexStr.substr(0, pos1)) - 1;
                    if (pos1 + 1 < vertexStr.length()) {
                        fv.texCoordIndex = std::stoi(vertexStr.substr(pos1 + 1)) - 1;
                    }
                }
                else {
                    // Vertex, texture, and normal index
                    fv.vertexIndex = std::stoi(vertexStr.substr(0, pos1)) - 1;
                    if (pos2 > pos1 + 1) {
                        fv.texCoordIndex = std::stoi(vertexStr.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                    }
                    if (vertexStr.length() > pos2 + 1) {
                        fv.normalIndex = std::stoi(vertexStr.substr(pos2 + 1)) - 1;
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