#pragma once

#include "scop.hpp"
#include <string>

void printUsage(const char *programName);
bool parseArguments(int argc, char **argv, OBJModel &model);