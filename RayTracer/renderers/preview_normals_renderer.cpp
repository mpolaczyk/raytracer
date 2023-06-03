#include "stdafx.h"

#include <ppl.h>

#include "math/materials.h"
#include "math/hittables.h"
#include "math/camera.h"
#include "processing/benchmark.h"

#include "preview_normals_renderer.h"

std::string preview_normals_renderer::get_name() const
{
  return "CPU Preview Normals";
}

void preview_normals_renderer::render()
{
  save_output = false;

  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(chunk_strategy_type::vertical_stripes, std::thread::hardware_concurrency() * 32, job_state.image_width, job_state.image_height, chunks);
  
  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](const chunk& ch) { render_chunk(ch); });
}

void preview_normals_renderer::render_chunk(const chunk& in_chunk)
{
  assert(job_state.scene_root != nullptr);
  assert(job_state.cam != nullptr);
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  benchmark::scope_counter benchmark_render_chunk(oss.str(), false);

  hittable* l = job_state.scene_root->lights[0];
  assert(l != nullptr);
  vec3 light = l->get_origin();

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      // Effectively it is a fragment shader
      vec3 pixel_color;

      float u = float(x) / (job_state.image_width - 1);
      float v = float(y) / (job_state.image_height - 1);
      ray r = job_state.cam->get_ray(u, v);
      hit_record h;

      if (job_state.scene_root->hit(r, 0.01f, math::infinity, h))
      {
        pixel_color = (h.normal + 1.0f) * .5f;
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