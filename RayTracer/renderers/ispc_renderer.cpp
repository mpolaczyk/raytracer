#include "stdafx.h"

#include <ppl.h>
#include <math.h>

#include "math/materials.h"

#include "ispc_renderer.h"

#include "renderers/trace_ray.h"

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
  chunk_generator::generate_chunks(chunk_strategy_type::horizontal_stripes, std::thread::hardware_concurrency(), job_state.image_width, job_state.image_height, chunks);

  concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
}


void ispc_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  vec3 resolution(job_state.image_width, job_state.image_height, 0.0f);

  ispc::render_chunk((const ispc::vec3&)resolution, (const ispc::chunk&)in_chunk);
}
