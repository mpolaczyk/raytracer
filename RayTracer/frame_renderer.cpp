#include "frame_renderer.h"

#include <functional>
#include <assert.h>
#include <windows.h>
#include <ppl.h>
#include <cstdio>
#include <vector>

#include "camera.h"
#include "common.h"
#include "bmp.h"
#include "benchmark.h"

frame_renderer::frame_renderer(uint32_t width, uint32_t height, camera* cam)
  : image_width(width), image_height(height), cam(cam)
{
  assert(cam != nullptr);

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
    return diffuse_bounce_brightness * ray_color(ray(rec.p, target - rec.p), world, diffuse_bounce - 1);
  }
  vec3 unit_direction = unit_vector(r.direction());
  float t = 0.5f * (unit_direction.y() + 1.0f);
  return (1.0f - t) * white + t * blue;
}

void frame_renderer::render(const hittable_list& world)
{
  assert(cam != nullptr);
  assert(img != nullptr);

  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(parallel_chunks_strategy, parallel_chunks_num, image_width, image_height, chunks);
  for (const auto& ch : chunks)
  {
    std::cout << "Chunk=" << ch.id << " x=" << ch.x << " y=" << ch.y << " size_x=" << ch.size_x << " size_y=" << ch.size_y << std::endl;
  }

  // Process chunks on parallel
  concurrency::parallel_for_each(begin(chunks), end(chunks), 
    [&](chunk ch)
    {
      char name[100];
      std::sprintf(name, "Thread=%d", ch.id);
      benchmark::scope_counter counter(name);

      for (uint32_t y = ch.y; y < ch.y+ch.size_y; ++y)
      {
        for (uint32_t x = ch.x; x < ch.x+ch.size_x; ++x)
        {
          color3 pixel_color;
          // Anti Aliasing done at the ray level, multiple rays for each pixel.
          for (uint32_t c = 0; c < AA_samples_per_pixel; c++)
          {
            float u = (float(x) + random_cache::get_float()) / (image_width - 1);
            float v = (float(y) + random_cache::get_float()) / (image_height - 1);
            ray r = cam->get_ray(u, v);
            pixel_color += ray_color(r, world, diffuse_max_bounce_num);
          }
          bmp::bmp_pixel p = bmp::bmp_pixel(pixel_color / (float)AA_samples_per_pixel);
          img->draw_pixel(x, y, &p);
        }
      }
    });
}

void frame_renderer::save(const char* file_name)
{
  img->save_to_file(file_name);
}