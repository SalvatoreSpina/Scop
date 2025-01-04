#include "ArgumentParser.hpp"

void Parser::printUsage(const char *programName) {
  std::cerr << "Usage: " << programName << " <path/to/your/model.obj>\n";
}

Parser::Parser(int argc, char **argv, OBJModel &model) {
  parseArguments(argc, argv, model);
}

bool Parser::getSuccess() const { return this->success; }

void Parser::parseArguments(int argc, char **argv, OBJModel &model) {
  // We expect exactly one argument for the path to the .obj file.
  if (argc != 2 && argc != 3) {
    printUsage(argv[0]);
    return;
  }

  model.textureName = (argc == 3) ? argv[2] : "white.bmp";

  // The user-provided .obj file path
  const std::string filePath = argv[1];
  model.objectName = filePath;
  if (filePath.substr(filePath.size() - 4) != ".obj") {
    std::cerr << "Invalid file extension. Please provide a .obj file.\n";
    return;
  }
  std::cerr << "Loading OBJ file: " << filePath << "\n";

  // Attempt to load the OBJ file
  if (!OBJLoader::loadOBJ(filePath, model)) {
    std::cerr << "Failed to load OBJ file.\n";
    return;
  }

  // If successful, print some stats
  std::cout << "Loaded OBJ file successfully!\n";
  std::cout << "Object Name:    " << model.objectName << "\n";
  std::cout << "Texture Name:   " << model.textureName << "\n";
  std::cout << "Vertices:       " << model.vertices.size() << "\n";
  std::cout << "Texture Coords: " << model.texCoords.size() << "\n";
  std::cout << "Normals:        " << model.normals.size() << "\n";
  std::cout << "Faces:          " << model.faces.size() << "\n";

  this->success = true;
}
