#pragma once

#include "vec3.h"

namespace bmp
{
  constexpr uint32_t BYTES_PER_PIXEL = 3; /// RGB
  constexpr uint32_t FILE_HEADER_SIZE = 14;
  constexpr uint32_t INFO_HEADER_SIZE = 40;

  struct bmp_pixel
  {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    bmp_pixel() {}
    bmp_pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
    bmp_pixel(const color3& color)
    {
      r = static_cast<uint8_t>(color.x() * 255.0f);
      g = static_cast<uint8_t>(color.y() * 255.0f);
      b = static_cast<uint8_t>(color.z() * 255.0f);
    }
  };


  struct bmp_image
  {
    bmp_image(uint32_t w, uint32_t h)
      : width(w), height(h)
    {
      buffer = (uint8_t*)malloc(width * height * BYTES_PER_PIXEL * sizeof(uint8_t));
    }

    ~bmp_image()
    {
      if (width > 0 && height > 0 && buffer != nullptr)
      {
        free(buffer);
      }
    }

    void draw_pixel(uint32_t x, uint32_t y, const bmp_pixel* p);
    void save_to_file(const char* image_file_name) const;

  private:

    uint8_t* create_file_header(uint32_t height, uint32_t stride) const;
    uint8_t* create_info_header(uint32_t height, uint32_t width) const;

    uint8_t* buffer = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  
};