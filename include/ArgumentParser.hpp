#pragma once

#include "OBJLoader.hpp"
#include "OBJModel.hpp"

#include <iostream>
#include <string>

class Parser {
private:
  bool success = false;

  /**
   * @brief Prints usage instructions for the program.
   *
   * @param programName Name of the executable.
   */
  void printUsage(const char *programName);

public:
  /**
   * @brief Constructor for the Parser class.
   *
   * @param argc   Number of command-line arguments.
   * @param argv   Command-line argument values.
   * @param model  Reference to an OBJModel to be filled on success.
   */
  Parser(int argc, char **argv, OBJModel &model);

  /**
   * @brief Gets the success state of the parsing process.
   *
   * @return true if parsing was successful, false otherwise.
   */
  bool getSuccess() const;

  /**
   * @brief Parses the command-line arguments, loads the OBJ model if valid.
   *
   * @param argc   Number of command-line arguments.
   * @param argv   Command-line argument values.
   * @param model  Reference to an OBJModel to be filled on success.
   */
  void parseArguments(int argc, char **argv, OBJModel &model);
};
