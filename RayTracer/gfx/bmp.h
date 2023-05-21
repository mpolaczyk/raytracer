#pragma once

#include "math/common.h"

namespace bmp
{
  constexpr uint32_t BYTES_PER_PIXEL = 4; /// RGBA (A not in use, only for storage)
  constexpr uint32_t FILE_HEADER_SIZE = 14;
  constexpr uint32_t INFO_HEADER_SIZE = 40;

  enum class bmp_format
  {
    rgba = 0,     // DXGI_FORMAT_R8G8B8A8
    bgra          // BMP file
  };

  struct bmp_pixel
  {
    uint8_t b = 0;
    uint8_t g = 0;
    uint8_t r = 0;
    uint8_t a = 255;
    bmp_pixel() = default;
    bmp_pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
    bmp_pixel(const vec3& color)
    {
      // Detect NaNs
      assert(!isnan(color.x));
      assert(!isnan(color.y));
      assert(!isnan(color.z));
      r = static_cast<uint8_t>(math::clamp(color.x * 255.0f, 0.0f, 255.0f));
      g = static_cast<uint8_t>(math::clamp(color.y * 255.0f, 0.0f, 255.0f));
      b = static_cast<uint8_t>(math::clamp(color.z * 255.0f, 0.0f, 255.0f));
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

    void draw_pixel(uint32_t x, uint32_t y, const bmp_pixel* p, bmp_format format = bmp_format::bgra);
    void save_to_file(const char* image_file_name) const;
    uint8_t* get_buffer() { return buffer; }
    uint32_t get_width() { return width; }
    uint32_t get_height() { return height; }

   private:
    uint8_t* create_file_header(uint32_t height, uint32_t stride) const;
    uint8_t* create_info_header(uint32_t height, uint32_t width) const;

    uint8_t* buffer = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    
  };

  
};