#include "chunk_generator.h"

void chunk_generator::generate_chunks(chunk_strategy strategy, int num, int image_width, int image_height, std::vector<chunk>& out_chunks)
{
  switch (strategy)
  {
  case chunk_strategy::vertical_stripes:
    return generate_rectangles(num, 1, image_width, image_height, out_chunks);
    break;
  case chunk_strategy::horizontal_stripes:
    return generate_rectangles(1, num, image_width, image_height, out_chunks);
    break;
  case chunk_strategy::rectangles:
    return generate_rectangles(sqrt(num), sqrt(num), image_width, image_height, out_chunks);
    break;
  default:
    break;
  }
}

void chunk_generator::generate_rectangles(int num_x, int num_y, int image_width, int image_height, std::vector<chunk>& out_chunks)
{
  int n = 0;
  for (int ny = 0; ny < num_y; ny++)
  {
    for (int nx = 0; nx < num_x; nx++)
    {
      chunk ch;
      ch.id = n;
      ch.size_x = image_width / num_x;
      ch.size_y = image_height / num_y;
      ch.x = num_x == 1 ? 0 : nx * ch.size_x;
      ch.y = num_y == 1 ? 0 : ny * ch.size_y;
      out_chunks.push_back(ch);
      n += 1;
    }
  }
}

