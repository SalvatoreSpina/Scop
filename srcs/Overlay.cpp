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
                     int totalModes) {
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
  float xPos = 10.0f;
  float yPos = m_height - lineHeight - 10; // Start near top

  // Display camera information line by line
  std::istringstream cameraStream(cameraInfo);
  std::string line;
  while (std::getline(cameraStream, line)) {
    drawText(xPos, yPos, line);
    yPos -= lineHeight;
  }

  // Display current rendering mode
  std::stringstream ss;
  ss << "Current Mode: " << currentMode << " / " << (totalModes - 1);
  drawText(xPos, yPos, ss.str());
  yPos -= lineHeight;

  // Instructions
  if (currentMode <
      static_cast<int>(RenderMode::COUNT)) { // Ensure mode is valid
    switch (static_cast<RenderMode>(currentMode)) {
    case RenderMode::GRAYSCALE:
      drawText(xPos, yPos, "Mode 0: Grayscale");
      break;
    case RenderMode::RANDOM_COLOR:
      drawText(xPos, yPos, "Mode 1: Random Color");
      break;
    case RenderMode::MATERIAL_COLOR:
      drawText(xPos, yPos, "Mode 2: Material Color");
      break;
    case RenderMode::TEXTURE:
      drawText(xPos, yPos, "Mode 3: Texture");
      break;
    default:
      break;
    }
  }
  yPos -= lineHeight;

  drawText(xPos, yPos, "Press 'T' to cycle modes.");
  yPos -= lineHeight;
  drawText(xPos, yPos, "Press 'F' to toggle Free Camera Mode.");
  yPos -= lineHeight;
  if (currentMode >= 0 && currentMode < static_cast<int>(RenderMode::COUNT)) {
    // Additional instructions based on mode can be added here if needed
  }

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
  glRasterPos2f(x, y);
  for (const char &c : text) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  }
}
