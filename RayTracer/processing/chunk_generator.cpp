#include "stdafx.h"

#include "chunk_generator.h"

void chunk_generator::generate_chunks(chunk_strategy_type strategy, uint32_t num, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks)
{
  uint32_t snum = (uint32_t)sqrt(num);
  switch (strategy)
  {
  case chunk_strategy_type::vertical_stripes:
    return generate_rectangles(num, 1, image_width, image_height, out_chunks);
    
  case chunk_strategy_type::horizontal_stripes:
    return generate_rectangles(1, num, image_width, image_height, out_chunks);
    
  case chunk_strategy_type::rectangles:
    if (snum * snum != num)
    {
      // Round to the nearest power of two https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
      snum--;
      snum |= snum >> 1;
      snum |= snum >> 2;
      snum |= snum >> 4;
      snum |= snum >> 8;
      snum |= snum >> 16;
      snum++;
      return generate_rectangles(snum, num / snum, image_width, image_height, out_chunks);
    }
    return generate_rectangles(snum, snum, image_width, image_height, out_chunks);

  default:
    return generate_rectangles(1, 1, image_width, image_height, out_chunks);
  }
}

void chunk_generator::generate_rectangles(uint32_t num_x, uint32_t num_y, uint32_t image_width, uint32_t image_height, std::vector<chunk>& out_chunks)
{
  logger::info("Grid num_x={0} num_y={1}", num_x, num_y);
  uint32_t n = 0;
  for (uint32_t ny = 0; ny < num_y; ny++)
  {
    for (uint32_t nx = 0; nx < num_x; nx++)
    {
      chunk ch;
      ch.id = n;
      // Divide axis as float, find range between (n, n+1) 
      ch.x = num_x == 1 ? 0 : (int)((float)nx * (float)image_width / (float)num_x);
      ch.y = num_y == 1 ? 0 : (int)((float)ny * (float)image_height / (float)num_y);;
      ch.size_x = ((int)((float)(nx + 1) * (float)image_width / (float)num_x) - ch.x);
      ch.size_y = ((int)((float)(ny + 1) * (float)image_height / (float)num_y) - ch.y);
      out_chunks.push_back(ch);
      n += 1;
    }
  }
}

