#include "Overlay.hpp"
#include <iostream>
#include <sstream>

static void *BIG_FONT = GLUT_BITMAP_HELVETICA_18;

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
  glDisable(GL_TEXTURE_2D);

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

  //
  // 1) Left side: Keybinds
  //
  float leftXPos = 40.0f;
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
  drawText(leftXPos, leftYPos, "Space: Reset camera (Focus Mode).");

  //
  // 2) Right side: Current camera / rendering data
  //
  float rightXPos = m_width - 300.0f;
  float rightYPos = m_height - padding;

  drawText(rightXPos, rightYPos, "Current Data:");
  rightYPos -= lineHeight;

  std::istringstream cameraStream(cameraInfo);
  std::string line;
  while (std::getline(cameraStream, line)) {
    drawText(rightXPos, rightYPos, line);
    rightYPos -= lineHeight;
  }

  static const char *modes[] = {"Grayscale", "Random Color", "Texture"};
  const char *currentModeName = modes[currentMode];
  std::stringstream ss;
  ss << "Render Mode: " << currentModeName << " (" << (currentMode + 1) << " / "
     << totalModes << ")";

  drawText(rightXPos, rightYPos, ss.str());

  //
  // 3) Bottom-left corner: Model details
  //
  float bottomLeftXPos = 40.0f;
  float bottomLeftYPos = 30.0f + lineHeight * 6;

  drawText(bottomLeftXPos, bottomLeftYPos, "Model Details:");
  bottomLeftYPos -= lineHeight;
  drawText(bottomLeftXPos, bottomLeftYPos, "Object Name: " + model.objectName);
  bottomLeftYPos -= lineHeight;
  drawText(bottomLeftXPos, bottomLeftYPos,
           "Texture Name: " + model.textureName);
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

  float openW = 120.0f;
  float openX = (float)m_width - openW - 80.0f;
  float openY = 40.0f;

  // Draw the bigger text
  drawLargeText(openX, openY, "Open assets folder");

  // Restore previous states
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
}

/**
 * @brief Draws standard-sized text at the specified (x, y) position
 *        using GLUT_BITMAP_9_BY_15.
 */
void Overlay::drawText(float x, float y, const std::string &text) {
  glColor3f(1.0f, 1.0f, 1.0f); // Always render text in white
  glRasterPos2f(x, y);
  for (char c : text) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  }
}

/**
 * @brief Draws larger text at (x, y) using a bigger GLUT font.
 *        e.g. GLUT_BITMAP_HELVETICA_18 or GLUT_BITMAP_TIMES_ROMAN_24
 */
void Overlay::drawLargeText(float x, float y, const std::string &text) {
  glColor3f(1.0f, 1.0f, 1.0f);
  glRasterPos2f(x, y);
  for (char c : text) {
    glutBitmapCharacter(BIG_FONT, c);
  }
}
