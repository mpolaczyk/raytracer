#include "stdafx.h"

#include <ppl.h>
#include <math.h>

#include "math/materials.h"
#include "processing/benchmark.h"

#include "ispc_renderer.h"

static_assert(sizeof(ispc::chunk) == sizeof(chunk));
static_assert(sizeof(ispc::float3) == sizeof(vec3));

std::string ispc_renderer::get_name() const
{
  return "CPU ISPC (Example only)";
}

void ispc_renderer::render()
{
  save_output = true;

  std::vector<chunk> chunks;
  const int chunks_per_thread = 32;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency(), job_state.image_width, job_state.image_height, chunks);

  ispc::float3* output = new ispc::float3[job_state.image_width * job_state.image_height];

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch, output); });

  // Convert from the output to bitmap - THIS IS STUPID
  int index = 0;
  for (int x = 0; x < job_state.image_width; ++x)
  {
    for (int y = 0; y < job_state.image_height; ++y)
    {
      ispc::float3 value = output[index];
      index++;
      vec3 pixel_color = vec3(value.v[0], value.v[1], value.v[2]);

      bmp::bmp_pixel p(pixel_color);
      job_state.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
      if (save_output)
      {
        job_state.img_bgr->draw_pixel(x, y, &p);
      }
    }
  }

  delete output;
}


void ispc_renderer::render_chunk(const chunk& in_chunk, ispc::float3* output)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  benchmark::scope_counter benchmark_render_chunk(oss.str(), false);

  vec3 resolution((float)job_state.image_width, (float)job_state.image_height, 0.0f);

  ispc::render_chunk((const ispc::float3&)resolution, (const ispc::chunk&)in_chunk, output);
}
