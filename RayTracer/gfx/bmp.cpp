#include "stdafx.h"

#include "bmp.h"

uint8_t* bmp::bmp_image::create_file_header(uint32_t height, uint32_t stride) const
{
  uint32_t file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

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

uint8_t* bmp::bmp_image::create_info_header(uint32_t height, uint32_t width) const
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

void bmp::bmp_image::draw_pixel(uint32_t x, uint32_t y, const bmp_pixel* p, bmp_format format)
{
  assert(x < width);
  assert(y < height);

  uint32_t pixel_addr = y * width * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;
  if (format == bmp_format::rgba)
  {
    bmp_pixel p2(p->b, p->g, p->r);
    memcpy(buffer + pixel_addr, &p2, sizeof(bmp_pixel));
  }
  else if (format == bmp_format::bgra)
  {
    memcpy(buffer + pixel_addr, p, sizeof(bmp_pixel));
  }
}

void bmp::bmp_image::save_to_file(const char* image_file_name) const
{
  assert(image_file_name != nullptr);
  const uint8_t padding[3] = { 0, 0, 0 };

  uint32_t width_in_bytes = width * BYTES_PER_PIXEL;
  uint32_t padding_size = (4 - (width_in_bytes) % 4) % 4;
  assert(padding_size <= 3);
  uint32_t stride = width_in_bytes + padding_size;

  const char* mode = "wb";
  FILE* image_file = nullptr;
  fopen_s(&image_file, image_file_name, mode);
  assert(image_file != nullptr);

  fwrite(create_file_header(height, stride), 1, FILE_HEADER_SIZE, image_file);
  fwrite(create_info_header(height, width), 1, INFO_HEADER_SIZE, image_file);

  if (padding_size == 0)
  {
    fwrite(buffer, BYTES_PER_PIXEL, height * width, image_file);
  }
  else
  {
    for (uint32_t x = 0; x < height; x++)
    {
      fwrite(buffer + (x * width_in_bytes), BYTES_PER_PIXEL, width, image_file);
      fwrite(padding, 1, padding_size, image_file);
    }
  }

  fclose(image_file);
}