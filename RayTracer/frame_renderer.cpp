#include "stdafx.h"

#include "frame_renderer.h"

#include <vector>
#include <ppl.h>

#include "bmp.h"
#include "thread_pool.h"

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

frame_renderer::frame_renderer(uint32_t width, uint32_t height, const renderer_settings& settings, const camera& cam)
  : settings(settings), image_width(width), image_height(height), cam(cam)
{
  img = new bmp::bmp_image(image_width, image_height);

  std::cout << "Frame renderer: " << image_width << "x" << image_height << std::endl;

  random_cache::init();
}
frame_renderer::~frame_renderer()
{
  if (img != nullptr)
  {
    delete img;
  }
}

void frame_renderer::render(const hittable_list& world)
{
  assert(cam != nullptr);
  assert(img != nullptr);

  // Build chunks of work
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(settings.chunks_strategy, settings.chunks_num, image_width, image_height, chunks);
  for (const auto& ch : chunks)
  {
    std::cout << "Chunk=" << ch.id << " x=" << ch.x << " y=" << ch.y << " size_x=" << ch.size_x << " size_y=" << ch.size_y << std::endl;
  }

  // Process chunks on parallel
  if (settings.threading_strategy == threading_strategy_type::none)
  {
    chunk ch;
    ch.id = 1;
    ch.size_x = image_width;
    ch.size_y = image_height;
    render_chunk(world, ch);
  }
  if (settings.threading_strategy == threading_strategy_type::thread_pool)
  {
    thread_pool pool;
    for (const auto& ch : chunks)
    {
      pool.queue_job([&]() { render_chunk(world, ch); });
    }
    pool.start(settings.threads_num);
    while (pool.is_busy()) { }
    pool.stop();
  }
  else if (settings.threading_strategy == threading_strategy_type::pll_for_each)
  {
    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(world, ch); });
  }
}

void frame_renderer::render_chunk(const hittable_list& world, const chunk& ch)
{
  std::thread::id thread_id = std::this_thread::get_id();
  char name[100];
  std::sprintf(name, "Thread=%d Chunk=%d", thread_id, ch.id);
  benchmark::scope_counter counter(name);

  for (uint32_t y = ch.y; y < ch.y + ch.size_y; ++y)
  {
    for (uint32_t x = ch.x; x < ch.x + ch.size_x; ++x)
    {
      color3 pixel_color;
      // Anti Aliasing done at the ray level, multiple rays for each pixel.
      for (uint32_t c = 0; c < settings.AA_samples_per_pixel; c++)
      {
        float u = (float(x) + random_cache::get_float()) / (image_width - 1);
        float v = (float(y) + random_cache::get_float()) / (image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, world, settings.diffuse_max_bounce_num);
      }
      bmp::bmp_pixel p = bmp::bmp_pixel(pixel_color / (float)settings.AA_samples_per_pixel);
      img->draw_pixel(x, y, &p);
    }
  }
}

color3 inline frame_renderer::ray_color(const ray& r, const hittable_list& world, uint32_t diffuse_bounce)
{
  if (diffuse_bounce <= 0)
  {
    return black;
  }

  hit_record rec;
  if (world.hit(r, 0.001f, infinity, rec))
  {
    // Surface hit, cast a bounced ray
    // Using cached random values is twice faster but can cause glitches if low sample size.
    // Both unit_vector calls can be removed too, some performance is restored, quality similar.
    point3 target = rec.p + rec.normal + unit_vector(random_cache::get_vec3());
    return settings.diffuse_bounce_brightness * ray_color(ray(rec.p, target - rec.p), world, diffuse_bounce - 1);
  }

  vec3 unit_direction = unit_vector(r.direction);
  float y =  0.5f * (unit_direction.y() + 1.0f); // base blend based on y component of a ray
  return (1.0 - y)* white + y * blue; // blend between white and blue
}

void frame_renderer::save(const char* file_name)
{
  img->save_to_file(file_name);
}