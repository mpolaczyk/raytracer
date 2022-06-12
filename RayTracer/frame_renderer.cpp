#include "stdafx.h"

#include "frame_renderer.h"

#include <vector>
#include <ppl.h>

#include "bmp.h"
#include "thread_pool.h"
#include "sphere.h"
#include "material.h"

// Designated initializers c++20 https://en.cppreference.com/w/cpp/language/aggregate_initialization
renderer_settings renderer_settings::high_quality_preset
{ 
  .AA_samples_per_pixel = 50,
  .diffuse_max_bounce_num = 20
};
renderer_settings renderer_settings::medium_quality_preset
{
  .AA_samples_per_pixel = 20,
  .diffuse_max_bounce_num = 10
};
renderer_settings renderer_settings::low_quality_preset
{
  .AA_samples_per_pixel = 5,
  .diffuse_max_bounce_num = 3
};

frame_renderer::frame_renderer(uint32_t width, uint32_t height, const renderer_settings& in_settings)
  : settings(in_settings), image_width(width), image_height(height)
{
  img = new bmp::bmp_image(image_width, image_height);

  std::cout << "Frame renderer: " << image_width << "x" << image_height << std::endl;
}
frame_renderer::~frame_renderer()
{
  if (img != nullptr)
  {
    delete img;
  }
}

void frame_renderer::set_camera(const camera& in_cam)
{
  cam = in_cam;
}

void frame_renderer::render_multiple(const sphere_list& in_world, const std::vector<std::pair<uint32_t, camera_setup>>& in_camera_states)
{
  camera cam;
  if (in_camera_states.size() < 2)
  {
    std::cout << "More than two camera states required" << std::endl;
    return;
  }
  for (int setup_id = 0; setup_id < in_camera_states.size() - 1; setup_id++)
  {
    int frame_begin = in_camera_states[setup_id].first;
    int frame_end = in_camera_states[setup_id + 1].first;
    camera_setup setup_begin = in_camera_states[setup_id].second;
    camera_setup setup_end = in_camera_states[setup_id + 1].second;

    for (int frame_id = frame_begin; frame_id < frame_end; frame_id++)
    {
      char name[100];
      std::sprintf(name, "setup_id=%d frame_id=%d", setup_id, frame_id);
      std::cout << name << std::endl;

      float f = (float)(frame_id - frame_begin) / (float)(frame_end - frame_begin);
      render_single(in_world, camera_setup::lerp(setup_begin, setup_end, f), frame_id);
    }
  }
}

void frame_renderer::render_single(const sphere_list& in_world, const camera_setup& in_camera_state, int frame_id)
{
  cam.set_camera(in_camera_state);
  {
    benchmark::scope_counter benchmark_render("Render");
    render(in_world);
  }

  char image_file_name[100];
  std::sprintf(image_file_name, "image_%d.bmp", frame_id);
  {
    benchmark::scope_counter benchmark_render("Save");
    save(image_file_name);
  }
}

void frame_renderer::render(const sphere_list& in_world)
{
  // Build chunks of work
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(settings.chunks_strategy, settings.chunks_num, image_width, image_height, chunks);
  //for (const auto& ch : chunks)
  //{
  //  std::cout << "Chunk=" << ch.id << " x=" << ch.x << " y=" << ch.y << " size_x=" << ch.size_x << " size_y=" << ch.size_y << std::endl;
  //}
  
  // Process chunks on parallel
  if (settings.threading_strategy == threading_strategy_type::none)
  {
    chunk ch;
    ch.id = 1;
    ch.size_x = image_width;
    ch.size_y = image_height;
    render_chunk(in_world, ch);
  }
  if (settings.threading_strategy == threading_strategy_type::thread_pool)
  {
    thread_pool pool;
    for (const auto& ch : chunks)
    {
      pool.queue_job([&]() { render_chunk(in_world, ch); });
    }
    pool.start(settings.threads_num > 0 ? settings.threads_num : std::thread::hardware_concurrency());
    while (pool.is_busy()) { }
    pool.stop();
  }
  else if (settings.threading_strategy == threading_strategy_type::pll_for_each)
  {
    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(in_world, ch); });
  }
}

void frame_renderer::render_chunk(const sphere_list& in_world, const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();
  char name[100];
  std::sprintf(name, "Thread=%d Chunk=%d", thread_id, in_chunk.id);
  benchmark::scope_counter benchmark_render_chunk(name, false);

  for (uint32_t y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (uint32_t x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      vec3 pixel_color;
      // Anti Aliasing done at the ray level, multiple rays for each pixel.
      for (uint32_t c = 0; c < settings.AA_samples_per_pixel; c++)
      {
        float u = (float(x) + random_cache::get_float()) / (image_width - 1);
        float v = (float(y) + random_cache::get_float()) / (image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, in_world, settings.background, settings.diffuse_max_bounce_num);
      }
      // Save to bmp
      bmp::bmp_pixel p = bmp::bmp_pixel(pixel_color / (float)settings.AA_samples_per_pixel);
      img->draw_pixel(x, y, &p);
    }
  }
}

vec3 inline frame_renderer::ray_color(const ray& in_ray, const sphere_list& in_world, const vec3& in_background, uint32_t depth)
{
  if (depth <= 0)
  {
    return black;
  }

  // NEW CODE FOR LIGHTING
  //hit_record hit;
  //// If the ray hits nothing, return the background color.
  //if (!in_world.hit(in_ray, 0.001, infinity, hit))
  //{
  //  return in_background;
  //}
  //
  //ray scattered;
  //vec3 attenuation;
  //vec3 emitted = hit.material->emitted(hit.u, hit.v, hit.p);
  //
  //if (!hit.material->scatter(in_ray, hit, attenuation, scattered))
  //{
  //  return emitted;
  //}
  //
  //return emitted + attenuation * ray_color(scattered, in_background, in_world, depth - 1);

  hit_record hit;
  if (in_world.hit(in_ray, 0.001f, infinity, hit))  // 0.001f to fix "shadow acne"
  {
    ray scattered;
    vec3 attenuation;
    if (hit.material->scatter(in_ray, hit, attenuation, scattered))
    {
      return attenuation * ray_color(scattered, in_world, in_background, depth - 1);
    }
    return black;
    
  }
  
  vec3 unit_direction = in_ray.direction;              // unit_vector(r.direction)
  float y =  0.5f * (unit_direction.y + 1.0f);  // base blend based on y component of a ray
  return (1.0f - y) * white + y * white_blue;     // linear blend (lerp) between white and blue
}

void frame_renderer::save(const char* file_name)
{
  img->save_to_file(file_name);
}