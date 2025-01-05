#include "TextureManager.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

GLuint TextureManager::loadBMPTexture(const std::string &filePath) {
  // Open the BMP file
  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open BMP file: " << filePath << std::endl;
    return 0;
  }

  // Read the BMP header (14 bytes)
  unsigned char fileHeader[14];
  file.read(reinterpret_cast<char *>(fileHeader), 14);
  if (file.gcount() != 14) {
    std::cerr << "Error: Failed to read BMP file header.\n";
    return 0;
  }

  // Verify the BMP signature
  if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    std::cerr << "Error: File is not a valid BMP: " << filePath << std::endl;
    return 0;
  }

  // Read the DIB header (BITMAPINFOHEADER - 40 bytes)
  unsigned char infoHeader[40];
  file.read(reinterpret_cast<char *>(infoHeader), 40);
  if (file.gcount() != 40) {
    std::cerr << "Error: Failed to read BMP info header.\n";
    return 0;
  }

  // Extract image dimensions and properties
  int width = *reinterpret_cast<int *>(&infoHeader[4]);
  int height = *reinterpret_cast<int *>(&infoHeader[8]);
  short planes = *reinterpret_cast<short *>(&infoHeader[12]);
  short bpp = *reinterpret_cast<short *>(&infoHeader[14]);
  int compression = *reinterpret_cast<int *>(&infoHeader[16]);

  // Validate BMP format
  if (planes != 1) {
    std::cerr << "Error: BMP planes (" << planes << ") is not 1.\n";
    return 0;
  }
  if (bpp != 24) {
    std::cerr << "Error: BMP bit depth (" << bpp << ") is not 24.\n";
    return 0;
  }
  if (compression != 0) {
    std::cerr << "Error: BMP compression (" << compression
              << ") is not supported.\n";
    return 0;
  }

  // Check if height is negative (top-down BMP)
  bool bottomUp = true;
  if (height < 0) {
    bottomUp = false;
    height = -height;
    std::cerr << "Warning: BMP is top-down. Processing as bottom-up.\n";
  }

  // Calculate row padding
  int rowSize =
      (width * 3 + 3) & (~3); // Each row padded to multiple of 4 bytes

  // Move to the pixel data offset (from file header)
  int dataOffset = *reinterpret_cast<int *>(&fileHeader[10]);
  file.seekg(dataOffset, std::ios::beg);

  // Read pixel data
  std::vector<unsigned char> bmpData(rowSize * height);
  file.read(reinterpret_cast<char *>(bmpData.data()), bmpData.size());
  if (file.gcount() != static_cast<std::streamsize>(bmpData.size())) {
    std::cerr << "Error: Failed to read BMP pixel data.\n";
    return 0;
  }
  file.close();

  // Convert BMP data (BGR) to RGB and flip vertically if needed
  std::vector<unsigned char> pixels(width * height * 3, 0);
  for (int y = 0; y < height; ++y) {
    int bmpY = bottomUp ? (height - 1 - y) : y;
    for (int x = 0; x < width; ++x) {
      int bmpIndex = y * rowSize + x * 3;
      int pixelIndex = (bmpY * width + x) * 3;

      // BMP stores pixels in BGR format
      pixels[pixelIndex + 0] = bmpData[bmpIndex + 2]; // R
      pixels[pixelIndex + 1] = bmpData[bmpIndex + 1]; // G
      pixels[pixelIndex + 2] = bmpData[bmpIndex + 0]; // B
    }
  }

  // Generate OpenGL texture
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data to GPU
  glTexImage2D(GL_TEXTURE_2D,
               0,                // mipmap level
               GL_RGB,           // internal format
               width, height,    // width and height
               0,                // border
               GL_RGB,           // format
               GL_UNSIGNED_BYTE, // data type
               pixels.data()     // pointer to data
  );

  std::cout << "Loaded BMP texture: " << filePath << " (Width: " << width
            << ", Height: " << height << ", Texture ID: " << texID << ")\n";

  return texID;
}

GLuint TextureManager::generateWhiteTexture(unsigned int width,
                                            unsigned int height) {
  // Create a white pixel array
  std::vector<unsigned char> whitePixels(width * height * 3, 255);

  // Generate OpenGL texture
  GLuint texID = 0;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload texture data to GPU
  glTexImage2D(GL_TEXTURE_2D,
               0,                   // mipmap level
               GL_RGB,              // internal format
               width, height,       // width and height
               0,                   // border
               GL_RGB,              // format
               GL_UNSIGNED_BYTE,    // data type
               whitePixels.data()); // pointer to data

  std::cout << "Generated a solid white texture (Texture ID: " << texID
            << ", Size: " << width << "x" << height << ")\n";

  return texID;
}
