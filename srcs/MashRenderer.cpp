#include "MeshRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

void MeshRenderer::drawAllFaces(
    const OBJModel &model, RenderMode mode, GLuint textureID,
    const std::vector<std::array<float, 3>> &faceGrayColors,
    const std::vector<std::array<float, 3>> &faceRandomColors,
    const std::vector<std::array<float, 3>> &faceMaterialColors) {
  // Bind and enable texture if in TEXTURE mode
  if (mode == RenderMode::TEXTURE && textureID != 0) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_TEXTURE_2D);
  } else {
    // Otherwise disable texturing
    glDisable(GL_TEXTURE_2D);
  }

  bool hasTexCoords = !model.texCoords.empty();

  // Handle WIRE_FRAME mode
  if (mode == RenderMode::WIRE_FRAME) {
    // Tell OpenGL to draw polygons as lines
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0f, 0.0f, 0.0f); // Black lines
  }

  // Iterate through each face
  for (size_t faceIndex = 0; faceIndex < model.faces.size(); ++faceIndex) {
    glBegin(GL_POLYGON);

    // Set face color if not in WIRE_FRAME mode
    if (mode != RenderMode::WIRE_FRAME) {
      setFaceColor(mode, faceIndex, faceGrayColors, faceRandomColors,
                   faceMaterialColors);
    }

    // Draw each vertex
    for (const auto &fv : model.faces[faceIndex].vertices) {
      if (fv.vertexIndex < 0 ||
          fv.vertexIndex >= static_cast<int>(model.vertices.size())) {
        continue; // Skip invalid indices
      }

      if (mode == RenderMode::TEXTURE) {
        if (hasTexCoords && fv.texCoordIndex >= 0 &&
            fv.texCoordIndex < static_cast<int>(model.texCoords.size())) {
          const auto &tc = model.texCoords[fv.texCoordIndex];
          glTexCoord2f(tc.u, 1.0f - tc.v); // Flip V
        } else {
          // Procedural planar mapping (e.g., on the XY plane)
          const auto &v = model.vertices[fv.vertexIndex];
          glTexCoord2f(v.x, v.y); // Adjust based on desired mapping
        }
      }

      // Specify vertex position
      const auto &v = model.vertices[fv.vertexIndex];
      glVertex3f(v.x, v.y, v.z);
    }

    glEnd();
  }

  // ------------------------------------------------------------------------
  // Restore polygon mode if we changed it
  // ------------------------------------------------------------------------
  if (mode == RenderMode::WIRE_FRAME) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  if (mode == RenderMode::TEXTURE && textureID != 0) {
    glDisable(GL_TEXTURE_2D);
  }
}

void MeshRenderer::setFaceColor(
    RenderMode mode, size_t faceIndex,
    const std::vector<std::array<float, 3>> &faceGrayColors,
    const std::vector<std::array<float, 3>> &faceRandomColors,
    const std::vector<std::array<float, 3>> &faceMaterialColors) {
  static_cast<void>(faceMaterialColors);

  switch (mode) {
  case RenderMode::GRAYSCALE: {
    const auto &gc = faceGrayColors[faceIndex];
    glColor3f(gc[0], gc[1], gc[2]);
    break;
  }
  case RenderMode::RANDOM_COLOR: {
    const auto &rc = faceRandomColors[faceIndex];
    glColor3f(rc[0], rc[1], rc[2]);
    break;
  }
  case RenderMode::TEXTURE:
    // White so as not to tint the texture
    glColor3f(1.0f, 1.0f, 1.0f);
    break;

  case RenderMode::WIRE_FRAME:
    // Wireframe = black contour
    glColor3f(0.0f, 0.0f, 0.0f);
    break;

  default:
    // Fallback
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  }
}
