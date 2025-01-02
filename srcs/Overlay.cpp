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

  // Display camera information
  drawText(xPos, yPos, cameraInfo);
  yPos -= lineHeight;

  // Display current rendering mode
  std::stringstream ss;
  ss << "Current Mode: " << currentMode << " / " << (totalModes - 1);
  drawText(xPos, yPos, ss.str());
  yPos -= lineHeight;

  // Instructions
  drawText(xPos, yPos, "Press 'T' to cycle modes.");
  yPos -= lineHeight;
  drawText(xPos, yPos, "Drag & drop a BMP texture onto the window.");

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
