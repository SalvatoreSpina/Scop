#pragma once

#include "OBJLoader.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <vector>

/**
 * @brief A helper class dedicated to drawing an OBJModel with various color
 * modes.
 */
class MeshRenderer {
public:
  /**
   * @brief Draws all faces of the model with the given render mode and texture.
   * @param model The OBJ model to draw.
   * @param mode The mode used to set the face color or texture.
   * @param textureID The texture ID (if mode == TEXTURE).
   * @param faceGrayColors Precomputed face-based grayscale colors.
   * @param faceRandomColors Precomputed face-based random colors.
   * @param faceMaterialColors Precomputed face-based material colors.
   */
  static void
  drawAllFaces(const OBJModel &model, RenderMode mode, GLuint textureID,
               const std::vector<std::array<float, 3>> &faceGrayColors,
               const std::vector<std::array<float, 3>> &faceRandomColors,
               const std::vector<std::array<float, 3>> &faceMaterialColors);

private:
  /**
   * @brief Sets the OpenGL face color based on the current rendering mode.
   */
  static void
  setFaceColor(RenderMode mode, size_t faceIndex,
               const std::vector<std::array<float, 3>> &faceGrayColors,
               const std::vector<std::array<float, 3>> &faceRandomColors,
               const std::vector<std::array<float, 3>> &faceMaterialColors);
};
