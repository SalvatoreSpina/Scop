#include "Overlay.hpp"
#include <iostream>
#include <sstream>

/**
 * @brief Constructs an Overlay with given window dimensions.
 */
Overlay::Overlay(int width, int height) : m_width(width), m_height(height) {}

/**
 * @brief Updates the window dimensions.
 */
void Overlay::updateWindowSize(int width, int height) {
  m_width = width;
  m_height = height;
}

/**
 * @brief Renders the overlay with provided camera information and current mode.
 */
void Overlay::render(const std::string &cameraInfo, int currentMode,
                     int totalModes, const OBJModel &model) {
  // Save current projection and modelview matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, m_width, 0, m_height, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);

  float lineHeight = 18.0f;
  float padding = 100.0f;

  // Left side: Keybinds and their effects
  float leftXPos = 10.0f;
  float leftYPos = m_height - padding;

  drawText(leftXPos, leftYPos, "Keybinds:");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos, "Press 'T' to cycle modes.");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos, "Press 'F' to toggle Free Camera Mode.");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos, "Arrow keys: Rotate (Free Camera Mode).");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos, "W/A/S/D/Q/E: Move (Free Camera Mode).");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos, "'+'/'-': Adjust rotation speed (Focus Mode).");
  leftYPos -= lineHeight;
  drawText(leftXPos, leftYPos,
           "Space: Reset camera and settings (Focus Mode).");

  // Right side: Current data
  float rightXPos = m_width - 300.0f;
  float rightYPos = m_height - padding;

  std::istringstream cameraStream(cameraInfo);
  std::string line;

  drawText(rightXPos, rightYPos, "Current Data:");
  rightYPos -= lineHeight;

  while (std::getline(cameraStream, line)) {
    drawText(rightXPos, rightYPos, line);
    rightYPos -= lineHeight;
  }

  std::stringstream ss;
  static const char *modes[] = {"Grayscale", "Random Color", "Texture"};
  const char *currentModeName = modes[currentMode];
  ss << "Render Mode: " << currentModeName << " (" << currentMode + 1 << " / "
     << totalModes << ")";
  drawText(rightXPos, rightYPos, ss.str());

  // Bottom-left corner: Model details
  float bottomLeftXPos = 10.0f;
  float bottomLeftYPos = 10.0f + lineHeight * 6; // Adjust for multi-line text

  drawText(bottomLeftXPos, bottomLeftYPos, "Model Details:");
  bottomLeftYPos -= lineHeight;

  drawText(bottomLeftXPos, bottomLeftYPos, "Object Name: " + model.objectName);
  bottomLeftYPos -= lineHeight;

  drawText(bottomLeftXPos, bottomLeftYPos, "Texture Name: " + model.textureName);
  bottomLeftYPos -= lineHeight;

  std::stringstream modelDetails;
  modelDetails << "Vertices: " << model.vertices.size();
  drawText(bottomLeftXPos, bottomLeftYPos, modelDetails.str());
  bottomLeftYPos -= lineHeight;

  modelDetails.str("");
  modelDetails << "Texture Coords: " << model.texCoords.size();
  drawText(bottomLeftXPos, bottomLeftYPos, modelDetails.str());
  bottomLeftYPos -= lineHeight;

  modelDetails.str("");
  modelDetails << "Normals: " << model.normals.size();
  drawText(bottomLeftXPos, bottomLeftYPos, modelDetails.str());
  bottomLeftYPos -= lineHeight;

  modelDetails.str("");
  modelDetails << "Faces: " << model.faces.size();
  drawText(bottomLeftXPos, bottomLeftYPos, modelDetails.str());

  // Restore previous states
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Draws text at the specified (x, y) position.
 */
void Overlay::drawText(float x, float y, const std::string &text) {
  glColor3f(1.0f, 1.0f, 1.0f); // Always render text in white
  glRasterPos2f(x, y);
  for (const char &c : text) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  }
}
