#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "vec3.h"

namespace bmp
{
  constexpr int BYTES_PER_PIXEL = 3; /// RGB
  constexpr int FILE_HEADER_SIZE = 14;
  constexpr int INFO_HEADER_SIZE = 40;

  uint8_t* create_file_header(int height, int stride)
  {
    int file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static uint8_t file_header[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    file_header[0] = (uint8_t)('B');
    file_header[1] = (uint8_t)('M');
    file_header[2] = (uint8_t)(file_size);
    file_header[3] = (uint8_t)(file_size >> 8);
    file_header[4] = (uint8_t)(file_size >> 16);
    file_header[5] = (uint8_t)(file_size >> 24);
    file_header[10] = (uint8_t)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return file_header;
  }

  uint8_t* create_info_header(int height, int width)
  {
    static uint8_t info_header[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    info_header[0] = (uint8_t)(INFO_HEADER_SIZE);
    info_header[4] = (uint8_t)(width);
    info_header[5] = (uint8_t)(width >> 8);
    info_header[6] = (uint8_t)(width >> 16);
    info_header[7] = (uint8_t)(width >> 24);
    info_header[8] = (uint8_t)(height);
    info_header[9] = (uint8_t)(height >> 8);
    info_header[10] = (uint8_t)(height >> 16);
    info_header[11] = (uint8_t)(height >> 24);
    info_header[12] = (uint8_t)(1);
    info_header[14] = (uint8_t)(BYTES_PER_PIXEL * 8);

    return info_header;
  }

  struct pixel
  {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    pixel() {}
    pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) { }
    pixel(const color3& color)
    {
      r = static_cast<uint8_t>(color.x() * 255.0f);
      g = static_cast<uint8_t>(color.y() * 255.0f);
      b = static_cast<uint8_t>(color.z() * 255.0f);
    }
  };

  template<int WIDTH,int HEIGHT>
  struct image
  {
    image()
    {
      buffer = (uint8_t*)malloc(WIDTH * HEIGHT * BYTES_PER_PIXEL * sizeof(uint8_t));
    }

    ~image()
    {
      free(buffer);
    }

    inline void draw_pixel(int x, int y, const pixel* p)
    {
      assert(x >= 0 && x < WIDTH);
      assert(y >= 0 && y < HEIGHT);
#if _DEBUG 
      assert(p->r >= 0 && p->r <= 255);
      assert(p->g >= 0 && p->g <= 255);
      assert(p->b >= 0 && p->b <= 255);
#endif
      int pixel_addr = y * WIDTH * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
      memcpy(buffer + pixel_addr, p, sizeof(pixel));
    }

    

    void save_to_file(char* image_file_name) const
    {
      assert(image_file_name != nullptr);
      const uint8_t padding[3] = { 0, 0, 0 };

      int width_in_bytes = WIDTH * BYTES_PER_PIXEL;
      int padding_size = (4 - (width_in_bytes) % 4) % 4;
      assert(padding_size >= 0 && padding_size <= 3);
      int stride = width_in_bytes+padding_size;

      const char* mode = "wb";
      FILE* image_file = nullptr;
      fopen_s(&image_file, image_file_name, mode);
      assert(image_file != nullptr);

      fwrite(create_file_header(HEIGHT, stride), 1, FILE_HEADER_SIZE, image_file);
      fwrite(create_info_header(HEIGHT, WIDTH), 1, INFO_HEADER_SIZE, image_file);

      if (padding_size == 0)
      {
        fwrite(buffer, BYTES_PER_PIXEL, HEIGHT * WIDTH, image_file);
      }
      else
      {
        for (int x = 0; x < HEIGHT; x++)
        {
          fwrite(buffer + (x * width_in_bytes), BYTES_PER_PIXEL, WIDTH, image_file);
          fwrite(padding, 1, padding_size, image_file);
        }
      }

      fclose(image_file);
    }

  private:
    uint8_t* buffer;
  };

  
};