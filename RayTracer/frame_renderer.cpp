#include "frame_renderer.h"

#include <functional>
#include <assert.h>
#include <windows.h>
#include <ppl.h>
#include <cstdio>

#include "camera.h"
#include "common.h"
#include "bmp.h"
#include "benchmark.h"

using namespace std;

frame_renderer::frame_renderer(int width, int height, camera* cam)
  : image_width(width), image_height(height), cam(cam)
{
  assert(cam != nullptr);

  img = new bmp::bmp_image(image_width, image_height);

  cout << "Frame renderer: " << image_width << "x" << image_height << endl;

  random_cache::init();
}
frame_renderer::~frame_renderer()
{
  if (img != nullptr)
  {
    delete img;
  }
}

color3 inline frame_renderer::ray_color(const ray& r, const hittable_list& world, int diffuse_bounce)
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

  struct chunk 
  {
    int id = 0;
    int x = 0;
    int y = 0;
    int size_x = 0;
    int size_y = 0;
  };
  // Chunks are vertical stripes
  std::vector<chunk> chunks;
  for (int n = 0; n < parallel_chunks_num; n++)
  {
    int desired_chunk_size_x = image_width / parallel_chunks_num;
    chunk ch;
    ch.id = n;
    ch.x = n * desired_chunk_size_x;
    ch.y = 0;
    if (ch.x + desired_chunk_size_x > image_width)
    {
      ch.size_x = ch.x + desired_chunk_size_x - image_width;
    }
    else
    {
      ch.size_x = desired_chunk_size_x;
    }
    ch.size_y = image_height;
    chunks.push_back(ch);
    std::cout << "Chunk=" << n << " x=" << ch.x << " y=" << ch.y << " size_x=" << ch.size_x << " size_y=" << ch.size_y << endl;
  }

  // Process chunks on parallel
  concurrency::parallel_for_each(begin(chunks), end(chunks), 
    [&](chunk ch)
    {
      char name[100];
      std::sprintf(name, "Thread=%d", ch.id);
      benchmark::scope_counter counter(name);

      for (int y = ch.y; y < ch.y+ch.size_y; ++y)
      {
        for (int x = ch.x; x < ch.x+ch.size_x; ++x)
        {
          color3 pixel_color;
          // Anti Aliasing done at the ray level, multiple rays for each pixel.
          for (int c = 0; c < AA_samples_per_pixel; c++)
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