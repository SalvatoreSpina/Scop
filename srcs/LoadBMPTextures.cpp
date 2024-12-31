#include <GL/glew.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

/**
 * @brief Loads a 24-bit uncompressed BMP file and creates an OpenGL texture
 * (GL_TEXTURE_2D).
 * @param filePath The path to the .bmp file on disk.
 * @return GLuint texture ID (0 if failure).
 */
GLuint LoadBMPTexture(const char *filePath) {
  // 1) Open file in binary mode
  std::ifstream file(filePath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open BMP file: " << filePath << std::endl;
    return 0;
  }

  // 2) Read the BITMAPFILEHEADER (14 bytes)
  //    - signature ("BM"), file size, reserved fields, data offset
  unsigned char fileHeader[14];
  file.read(reinterpret_cast<char *>(fileHeader), 14);

  // Check signature "BM"
  if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    std::cerr << "Not a valid BMP file (no 'BM' header)\n";
    return 0;
  }

  // The data offset is at fileHeader[10..13]
  std::uint32_t dataOffset =
      *reinterpret_cast<std::uint32_t *>(&fileHeader[10]);

  // 3) Read the BITMAPINFOHEADER (at least 40 bytes)
  //    - size of this header, width, height, planes, bpp, compression, etc.
  unsigned char infoHeader[40];
  file.read(reinterpret_cast<char *>(infoHeader), 40);

  // The size of this header is at infoHeader[0..3]
  std::uint32_t headerSize = *reinterpret_cast<std::uint32_t *>(&infoHeader[0]);
  // The image width is at infoHeader[4..7]
  std::int32_t width = *reinterpret_cast<std::int32_t *>(&infoHeader[4]);
  // The image height is at infoHeader[8..11]
  std::int32_t height = *reinterpret_cast<std::int32_t *>(&infoHeader[8]);
  // The number of planes is at infoHeader[12..13] (must be 1)
  std::uint16_t planes = *reinterpret_cast<std::uint16_t *>(&infoHeader[12]);
  // The bits per pixel is at infoHeader[14..15]
  std::uint16_t bpp = *reinterpret_cast<std::uint16_t *>(&infoHeader[14]);
  // The compression is at infoHeader[16..19] (we expect 0 for uncompressed)
  std::uint32_t compression =
      *reinterpret_cast<std::uint32_t *>(&infoHeader[16]);

  // Basic validations
  if (planes != 1) {
    std::cerr << "BMP has planes=" << planes << " (must be 1)\n";
    return 0;
  }
  if (bpp != 24) {
    std::cerr << "BMP is " << bpp << " bits per pixel (must be 24)\n";
    return 0;
  }
  if (compression != 0) {
    std::cerr << "BMP is compressed (compression=" << compression
              << "), not supported\n";
    return 0;
  }

  bool bottomUp = true;
  if (height < 0) {
    // Some BMPs can store top-down. We'll read them but won't flip in this
    // example. If you want, you can handle that by reading the absolute value
    // of height
    bottomUp = false;
    height = -height;
    std::cerr << "Warning: BMP is top-down (height < 0). We'll treat it as "
                 "bottom-up.\n";
  }

  // 4) Move file pointer to the pixel array
  file.seekg(dataOffset, std::ios::beg);

  // 5) Each row is padded to a multiple of 4 bytes
  //    For a 24-bit BMP, each pixel is 3 bytes. So rowSize is:
  std::uint32_t rowSize = ((24 * width + 31) / 32) * 4; // in bytes

  // Total data size
  std::uint32_t dataSize = rowSize * height;

  // 6) Read the raw pixel data
  std::vector<unsigned char> bmpData(dataSize);
  file.read(reinterpret_cast<char *>(bmpData.data()), dataSize);
  file.close();

  // 7) We must convert from BGR to RGB in each pixel
  //    Also handle that rows might be stored bottom-up
  //    If bottom-up, the first row in bmpData is the bottom row of the image
  //    We'll create a new buffer 'pixels' in standard (top-left first row)
  //    order and swapped to RGB.
  std::vector<unsigned char> pixels(width * height * 3);

  for (int y = 0; y < height; ++y) {
    // row in BMP data
    // If bottom-up, row 0 in file data is actually the bottom row
    // so the top row is row (height-1-y).
    int bmpRow = bottomUp ? (height - 1 - y) : y;

    // offset in the BMP file for that row
    std::uint32_t bmpOffset = bmpRow * rowSize;

    // offset in the final 'pixels' buffer for row y (top=0, down=height-1)
    std::uint32_t outOffset = (y * width * 3);

    for (int x = 0; x < width; ++x) {
      // b, g, r are consecutive
      unsigned char b = bmpData[bmpOffset + x * 3 + 0];
      unsigned char g = bmpData[bmpOffset + x * 3 + 1];
      unsigned char r = bmpData[bmpOffset + x * 3 + 2];

      // store in outOffset with R,G,B
      pixels[outOffset + x * 3 + 0] = r;
      pixels[outOffset + x * 3 + 1] = g;
      pixels[outOffset + x * 3 + 2] = b;
    }
  }

  // 8) Create an OpenGL texture
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);

  // Basic texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // 9) Upload the pixel data. We have an RGB 8-bit image:
  glTexImage2D(GL_TEXTURE_2D,
               0,                // level
               GL_RGB,           // internal format
               width, height,    // width, height
               0,                // border
               GL_RGB,           // format
               GL_UNSIGNED_BYTE, // type
               pixels.data());   // pointer to data

  // Optional: generate mipmaps
  // glGenerateMipmap(GL_TEXTURE_2D);

  std::cout << "BMP loaded: " << filePath << " (w=" << width << ", h=" << height
            << "), texID=" << texID << "\n";

  return texID;
}
