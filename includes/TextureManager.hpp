#pragma once

#include <string>

#include "OBJModel.hpp"
#include <GLFW/glfw3.h>

/**
 * @brief TextureManager is responsible for loading textures (e.g., BMP files),
 *        generating fallback textures, and returning OpenGL texture IDs.
 */
class TextureManager {
public:
  /**
   * @brief Loads a BMP texture from file and returns its OpenGL texture ID.
   * @param filePath The path to the .bmp file.
   * @return GLuint OpenGL texture ID (0 if loading failed).
   */
  static GLuint loadBMPTexture(const std::string &filePath);

  /**
   * @brief Generates a solid-white texture of the specified size and returns
   *        its OpenGL texture ID.
   * @param width The width of the texture in pixels.
   * @param height The height of the texture in pixels.
   * @return GLuint OpenGL texture ID.
   */
  static GLuint generateWhiteTexture(unsigned int width, unsigned int height);

private:
  TextureManager() = default; // Disallow instantiation
};
