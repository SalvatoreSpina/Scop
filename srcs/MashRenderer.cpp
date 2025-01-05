#include "MeshRenderer.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

void MeshRenderer::drawAllFaces(const OBJModel &model,
                                RenderMode mode,
                                GLuint textureID,
                                const std::vector<std::array<float, 3>> &faceGrayColors,
                                const std::vector<std::array<float, 3>> &faceRandomColors,
                                const std::vector<std::array<float, 3>> &faceMaterialColors) {
  if (mode == RenderMode::TEXTURE && textureID != 0) {
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_TEXTURE_2D);
  } else {
    // Disable texturing
    glDisable(GL_TEXTURE_2D);
  }

  // Iterate through each face
  for (size_t faceIndex = 0; faceIndex < model.faces.size(); ++faceIndex) {
    glBegin(GL_POLYGON);

    // Set the color for the entire face
    setFaceColor(mode, faceIndex,
                 faceGrayColors,
                 faceRandomColors,
                 faceMaterialColors);

    // Draw each vertex
    for (const auto &fv : model.faces[faceIndex].vertices) {
      if (fv.vertexIndex < 0 ||
          fv.vertexIndex >= static_cast<int>(model.vertices.size())) {
        continue; // skip invalid
      }

      if (mode == RenderMode::TEXTURE && fv.texCoordIndex >= 0 &&
          fv.texCoordIndex < static_cast<int>(model.texCoords.size())) {
        const auto &tc = model.texCoords[fv.texCoordIndex];
        glTexCoord2f(tc.u, 1.0f - tc.v); // Flip V
      }

      const auto &v = model.vertices[fv.vertexIndex];
      glVertex3f(v.x, v.y, v.z);
    }

    glEnd();
  }
}

void MeshRenderer::setFaceColor(RenderMode mode,
                                size_t faceIndex,
                                const std::vector<std::array<float, 3>> &faceGrayColors,
                                const std::vector<std::array<float, 3>> &faceRandomColors,
                                const std::vector<std::array<float, 3>> &faceMaterialColors) {
static_cast<void>(faceMaterialColors); // Unused
  switch (mode) {
  case RenderMode::GRAYSCALE:
  {
    const auto &gc = faceGrayColors[faceIndex];
    glColor3f(gc[0], gc[1], gc[2]);
    break;
  }
  case RenderMode::RANDOM_COLOR:
  {
    const auto &rc = faceRandomColors[faceIndex];
    glColor3f(rc[0], rc[1], rc[2]);
    break;
  }
  case RenderMode::TEXTURE:
    // White to not tint the texture
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  default:
    // Fallback
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  }
}
