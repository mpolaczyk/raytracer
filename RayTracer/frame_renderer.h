#pragma once

#include "vec3.h"
#include "hittable.h"
#include "ray.h"
#include "chunk_generator.h"

namespace bmp
{
  struct bmp_image;
}
class camera;

class frame_renderer
{
  // Image
  float aspect_ratio;
  uint32_t image_height;
  uint32_t image_width;

  // Camera
  float viewport_height;
  float viewport_width;
  float focal_length;

  bmp::bmp_image* img = nullptr;
  camera* cam = nullptr;

  uint32_t AA_samples_per_pixel = 50;            // Anti Aliasing oversampling
  uint32_t diffuse_max_bounce_num = 10;          // Diffuse bounce number
  float diffuse_bounce_brightness = 0.5f;

  uint32_t parallel_chunks_num = 8;
  chunk_strategy parallel_chunks_strategy = chunk_strategy::rectangles;

public:
  frame_renderer(uint32_t width, uint32_t height, camera* cam);
  ~frame_renderer();

  color3 ray_color(const ray& r, const hittable_list& world, uint32_t depth);
  void render(const hittable_list& world);
  void save(const char* file_name);
};