#include "stdafx.h"

#include <ppl.h>
#include <math.h>

#include "math/materials.h"

#include "ispc_renderer.h"

static_assert(sizeof(ispc::chunk) == sizeof(chunk));
static_assert(sizeof(ispc::vec3) == sizeof(vec3));

std::string ispc_renderer::get_name() const
{
  return "CPU ISPC";
}

void ispc_renderer::render()
{
  save_output = true;

  std::vector<chunk> chunks;
  const int chunks_per_thread = 32;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency(), job_state.image_width, job_state.image_height, chunks);

  ispc::vec3* output = new ispc::vec3[job_state.image_width * job_state.image_height];

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch, output); });

  // Convert from the output to bitmap - THIS IS STUPID
  int index = 0;
  for (int x = 0; x < job_state.image_width; ++x)
  {
    for (int y = 0; y < job_state.image_height; ++y)
    {
      ispc::vec3 value = output[index];
      index++;
      vec3 pixel_color = vec3(value.x, value.y, value.z);

      bmp::bmp_pixel p(pixel_color);
      job_state.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
    }
  }

  delete output;
}


void ispc_renderer::render_chunk(const chunk& in_chunk, ispc::vec3* output)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();

  benchmark::scope_counter benchmark_render_chunk(name, false);

  vec3 resolution(job_state.image_width, job_state.image_height, 0.0f);

  ispc::render_chunk((const ispc::vec3&)resolution, (const ispc::chunk&)in_chunk, output);
}
