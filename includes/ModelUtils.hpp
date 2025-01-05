#pragma once

#include "OBJLoader.hpp"
#include <vector>
#include <array>

/**
 * @brief A utility class for computing bounding boxes, centers, face colors,
 *        and other geometry-related tasks.
 */
class ModelUtilities {
public:
  /**
   * @brief Computes the bounding box of an OBJModel and returns:
   *        (minX, maxX, minY, maxY, minZ, maxZ).
   */
  static void computeBoundingBox(const OBJModel &model,
                                 float &minX, float &maxX,
                                 float &minY, float &maxY,
                                 float &minZ, float &maxZ);

  /**
   * @brief Computes the center of the model and returns it as (cx, cy, cz).
   */
  static void computeModelCenter(const OBJModel &model,
                                 float &cx, float &cy, float &cz);

  /**
   * @brief Builds per-face grayscale, random, and material colors.
   * @param model The OBJ model to process.
   * @param faceGrayColors Output vector of face-based grayscale colors.
   * @param faceRandomColors Output vector of face-based random colors.
   * @param faceMaterialColors Output vector of face-based "material" colors.
   */
  static void buildFaceBasedColors(const OBJModel &model,
                                   std::vector<std::array<float, 3>> &faceGrayColors,
                                   std::vector<std::array<float, 3>> &faceRandomColors,
                                   std::vector<std::array<float, 3>> &faceMaterialColors);
};
