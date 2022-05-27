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
  int id = 0;
  int x = 0;
  int y = 0;
  int size_x = 0;
  int size_y = 0;
};

struct chunk_generator
{
  static void generate_chunks(chunk_strategy strategy, int num, int image_width, int image_height, std::vector<chunk>& out_chunks);

private: 
  static void generate_rectangles(int num_x, int num_y, int image_width, int image_height, std::vector<chunk>& out_chunks);
};