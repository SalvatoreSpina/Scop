#include "scop.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Helper function that parses a single face vertex specification (e.g., "1", "1/2", "1//3", or "1/2/3").
 */
static FaceVertex parseFaceVertex(const std::string& vertexStr) {
    FaceVertex fv = {-1, -1, -1};

    // Locate the first slash
    size_t pos1 = vertexStr.find('/');
    if (pos1 == std::string::npos) {
        // Format: "v"
        // Only the vertex index is provided.
        fv.vertexIndex = std::stoi(vertexStr) - 1;
        return fv;
    }

    // There's at least one slash. Grab the vertex index first.
    fv.vertexIndex = std::stoi(vertexStr.substr(0, pos1)) - 1;

    // Now find the second slash, if any.
    size_t pos2 = vertexStr.find('/', pos1 + 1);
    if (pos2 == std::string::npos) {
        // Format: "v/t"
        // Only the texture coordinate is after the first slash.
        std::string texPart = vertexStr.substr(pos1 + 1);
        if (!texPart.empty()) {
            fv.texCoordIndex = std::stoi(texPart) - 1;
        }
    } else {
        // We have a second slash. Could be "v//n" or "v/t/n".
        if (pos2 == pos1 + 1) {
            // Format: "v//n" => no texture coordinate
            std::string normPart = vertexStr.substr(pos2 + 1);
            if (!normPart.empty()) {
                fv.normalIndex = std::stoi(normPart) - 1;
            }
        } else {
            // Format: "v/t/n"
            // Extract texture coordinate
            std::string texPart = vertexStr.substr(pos1 + 1, pos2 - (pos1 + 1));
            if (!texPart.empty()) {
                fv.texCoordIndex = std::stoi(texPart) - 1;
            }
            // Extract normal index
            std::string normPart = vertexStr.substr(pos2 + 1);
            if (!normPart.empty()) {
                fv.normalIndex = std::stoi(normPart) - 1;
            }
        }
    }

    return fv;
}

/**
 * Parses a line that starts with the "v" prefix (vertex position).
 */
static void parseVertex(std::istringstream& ss, OBJModel& model) {
    Vertex vertex;
    ss >> vertex.x >> vertex.y >> vertex.z;
    model.vertices.push_back(vertex);
}

/**
 * Parses a line that starts with the "vt" prefix (texture coordinate).
 */
static void parseTexCoord(std::istringstream& ss, OBJModel& model) {
    TexCoord texCoord;
    ss >> texCoord.u >> texCoord.v;
    // 'w' is optional in OBJ
    if (!(ss >> texCoord.w)) {
        texCoord.w = 0.0f;
    }
    model.texCoords.push_back(texCoord);
}

/**
 * Parses a line that starts with the "vn" prefix (vertex normal).
 */
static void parseNormal(std::istringstream& ss, OBJModel& model) {
    Normal normal;
    ss >> normal.x >> normal.y >> normal.z;
    model.normals.push_back(normal);
}

/**
 * Parses a line that starts with the "f" prefix (face data).
 */
static void parseFace(std::istringstream& ss, OBJModel& model) {
    Face face;
    std::string vertexStr;
    while (ss >> vertexStr) {
        face.vertices.push_back(parseFaceVertex(vertexStr));
    }
    model.faces.push_back(face);
}

/**
 * Dispatches a line to the appropriate parse function based on its prefix.
 * Returns false if the line is empty or commented out, true otherwise.
 */
static bool parseLine(const std::string& line, OBJModel& model) {
    // Trim leading/trailing spaces by reading through a stringstream
    std::istringstream ss(line);

    std::string prefix;
    ss >> prefix;

    // Skip empty lines or commented lines
    if (prefix.empty() || prefix[0] == '#') {
        return false;
    }

    // Dispatch to specialized parse functions
    if (prefix == "v") {
        parseVertex(ss, model);
    } 
    else if (prefix == "vt") {
        parseTexCoord(ss, model);
    } 
    else if (prefix == "vn") {
        parseNormal(ss, model);
    } 
    else if (prefix == "f") {
        parseFace(ss, model);
    }
    else {
        // Optional: Handle other OBJ elements (e.g., "g", "usemtl", "o", etc.)
        // or just ignore them
    }

    return true;
}

/**
 * Attempts to load an .obj file from the given path into the provided OBJModel.
 * Returns true on success, false otherwise.
 */
bool loadOBJ(const std::string &filePath, OBJModel &model) {
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
