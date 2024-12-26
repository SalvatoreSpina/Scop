#include "parsing.hpp"
#include "obj_parser.hpp"
#include "scop.hpp"

#include <iostream>

/**
 * @brief Prints usage instructions for the program.
 *
 * @param programName Name of the executable.
 */
void printUsage(const char *programName) {
  std::cerr << "Usage: " << programName << " <path/to/your/model.obj>\n";
}

/**
 * @brief Parses the command-line arguments, loads the OBJ model if valid.
 *
 * @param argc   Number of command-line arguments.
 * @param argv   Command-line argument values.
 * @param model  Reference to an OBJModel to be filled on success.
 * @return true  If the model is loaded successfully.
 * @return false Otherwise.
 */
bool parseArguments(int argc, char **argv, OBJModel &model) {
  // We expect exactly one argument for the path to the .obj file.
  if (argc != 2) {
    printUsage(argv[0]);
    return false;
  }

  // The user-provided .obj file path
  const std::string filePath = argv[1];
  std::cerr << "Loading OBJ file: " << filePath << "\n";

  // Attempt to load the OBJ file
  if (!loadOBJ(filePath, model)) {
    std::cerr << "Failed to load OBJ file.\n";
    return false;
  }

  // If successful, print some stats
  std::cout << "Loaded OBJ file successfully!\n";
  std::cout << "Vertices:       " << model.vertices.size() << "\n";
  std::cout << "Texture Coords: " << model.texCoords.size() << "\n";
  std::cout << "Normals:        " << model.normals.size() << "\n";
  std::cout << "Faces:          " << model.faces.size() << "\n";

  return true;
}