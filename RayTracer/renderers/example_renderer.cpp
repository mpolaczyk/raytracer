#include "stdafx.h"

#include <ppl.h>

#include "example_renderer.h"

std::string example_renderer::get_name() const
{
  return "CPU Example";
}

void example_renderer::render()
{
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency(), job_state.image_width, job_state.image_height, chunks);

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch); });
}

void example_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  benchmark::scope_counter benchmark_render_chunk(oss.str(), false);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      // Effectively it is a fragment shader
      vec3 pixel_color;

      int option = 3;
      if (option == 1)  // UV map
      {
        float u = float(x) / (job_state.image_width - 1);
        float v = float(y) / (job_state.image_height - 1);
        pixel_color = vec3(u, v, 0.5f);
      }
      else if (option == 2) // Procedural texture
      {
        pixel_color = vec3(abs(sin(x / 50.0f)), abs(sin(y / 300.0f)), 0.5f);
      }
      else if (option == 3) // Random noise
      {
        if (x < job_state.image_width / 2)
        {
          // Cached noise
          pixel_color = random_cache::direction();
        }
        else
        {
          // Runtime noise
          pixel_color = random_seed::direction(job_state.image_width * job_state.image_height);
        }
      }

      bmp::bmp_pixel p(pixel_color);
      job_state.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        job_state.img_bgr->draw_pixel(x, y, &p);
      }
    }
  }
}