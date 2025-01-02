#pragma once

#include <GL/freeglut.h>
#include <string>

/**
 * @brief Different ways to color or texture the 3D model.
 */
enum class RenderMode {
  GRAYSCALE = 0,
  RANDOM_COLOR,
  TEXTURE,
  COUNT // Not a mode, just to help us cycle
};

/**
 * @brief Handles rendering of overlay text (HUD) on the screen.
 */
class Overlay {
public:
  /**
   * @brief Constructs an Overlay with window dimensions.
   * @param width Window width in pixels.
   * @param height Window height in pixels.
   */
  Overlay(int width, int height);

  /**
   * @brief Updates the window dimensions.
   * @param width New window width.
   * @param height New window height.
   */
  void updateWindowSize(int width, int height);

  /**
   * @brief Renders the overlay text.
   * @param cameraInfo Information about the camera to display.
   * @param currentMode Current rendering mode.
   * @param totalModes Total number of rendering modes.
   */
  void render(const std::string &cameraInfo, int currentMode, int totalModes);

private:
  int m_width;  ///< Window width.
  int m_height; ///< Window height.

  /**
   * @brief Draws a single line of text at the specified position.
   * @param x X-coordinate in pixels.
   * @param y Y-coordinate in pixels.
   * @param text Text to render.
   */
  void drawText(float x, float y, const std::string &text);
};
