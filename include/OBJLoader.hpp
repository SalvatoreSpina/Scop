#pragma once

#include "OBJModel.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class OBJLoader {
private:
  /**
   * @brief Parses a single face vertex specification (e.g., "1", "1/2", "1//3",
   * or "1/2/3").
   */
  static FaceVertex parseFaceVertex(const std::string &vertexStr);

  /**
   * @brief Parses a line that starts with the "v" prefix (vertex position).
   */
  static void parseVertex(std::istringstream &ss, OBJModel &model);

  /**
   * @brief Parses a line that starts with the "vt" prefix (texture coordinate).
   */
  static void parseTexCoord(std::istringstream &ss, OBJModel &model);

  /**
   * @brief Parses a line that starts with the "vn" prefix (vertex normal).
   */
  static void parseNormal(std::istringstream &ss, OBJModel &model);

  /**
   * @brief Parses a line that starts with the "f" prefix (face data).
   */
  static void parseFace(std::istringstream &ss, OBJModel &model);

  /**
   * @brief Dispatches a line to the appropriate parse function based on its
   * prefix.
   */
  static bool parseLine(const std::string &line, OBJModel &model);

public:
  /**
   * @brief Loads an .obj file from the given path into the provided OBJModel.
   * @param filePath Path to the .obj file.
   * @param model Reference to an OBJModel to store the parsed data.
   * @return true if the file was loaded successfully, false otherwise.
   */
  static bool loadOBJ(const std::string &filePath, OBJModel &model);
};