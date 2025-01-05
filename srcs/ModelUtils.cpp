#include "ModelUtils.hpp"

#include <cfloat> // for FLT_MAX
#include <random>

void ModelUtilities::computeBoundingBox(const OBJModel &model, float &minX,
                                        float &maxX, float &minY, float &maxY,
                                        float &minZ, float &maxZ) {
  if (model.vertices.empty()) {
    // Return some default if empty
    minX = minY = minZ = 0.0f;
    maxX = maxY = maxZ = 0.0f;
    return;
  }

  minX = minY = minZ = FLT_MAX;
  maxX = maxY = maxZ = -FLT_MAX;

  for (const auto &v : model.vertices) {
    if (v.x < minX)
      minX = v.x;
    if (v.x > maxX)
      maxX = v.x;
    if (v.y < minY)
      minY = v.y;
    if (v.y > maxY)
      maxY = v.y;
    if (v.z < minZ)
      minZ = v.z;
    if (v.z > maxZ)
      maxZ = v.z;
  }
}

void ModelUtilities::computeModelCenter(const OBJModel &model, float &cx,
                                        float &cy, float &cz) {
  float minX, maxX, minY, maxY, minZ, maxZ;
  computeBoundingBox(model, minX, maxX, minY, maxY, minZ, maxZ);

  cx = 0.5f * (minX + maxX);
  cy = 0.5f * (minY + maxY);
  cz = 0.5f * (minZ + maxZ);
}

void ModelUtilities::buildFaceBasedColors(
    const OBJModel &model, std::vector<std::array<float, 3>> &faceGrayColors,
    std::vector<std::array<float, 3>> &faceRandomColors,
    std::vector<std::array<float, 3>> &faceMaterialColors) {
  faceGrayColors.resize(model.faces.size());
  faceRandomColors.resize(model.faces.size());
  faceMaterialColors.resize(model.faces.size());

  std::mt19937 rng(12345);
  std::uniform_real_distribution<float> dist(0.2f, 0.7f);

  for (size_t i = 0; i < model.faces.size(); ++i) {
    // Grayscale color
    float grey = dist(rng);
    faceGrayColors[i] = {grey, grey, grey};

    // Random color
    float r = dist(rng);
    float g = dist(rng);
    float b = dist(rng);
    faceRandomColors[i] = {r, g, b};

    // Material color (light blue in this example)
    faceMaterialColors[i] = {0.3f, 0.6f, 1.0f};
  }
}
