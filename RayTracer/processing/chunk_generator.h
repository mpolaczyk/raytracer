#pragma once

enum class chunk_strategy_type
{
  none = 0,
  vertical_stripes,
  horizontal_stripes,
  rectangles,
};
static inline const char* chunk_strategy_names[] = 
{ 
  "None", 
  "Vertical stripes", 
  "Horizontal stripes", 
  "Rectangles" 
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
  static void generate_chunks(chunk_strategy_type strategy, uint32_t num, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks);

private: 
  static void generate_rectangles(uint32_t num_x, uint32_t num_y, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks);
};