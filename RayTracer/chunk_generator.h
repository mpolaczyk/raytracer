#pragma once

#include <vector>

enum class chunk_strategy
{
  none = 0,
  vertical_stripes,
  horizontal_stripes,
  rectangles,
};

struct chunk
{
  uint32_t id = 0;
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t size_x = 0;
  uint32_t size_y = 0;
};

struct chunk_generator
{
  static void generate_chunks(chunk_strategy strategy, uint32_t num, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks);

private: 
  static void generate_rectangles(uint32_t num_x, uint32_t num_y, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks);
};