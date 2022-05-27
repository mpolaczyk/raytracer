#pragma once
#include <vector>
#include "vec3.h"
#include "hittable.h"
#include "ray.h"

namespace bmp
{
  struct bmp_image;
}
class camera;

class frame_renderer
{
  // Image
  float aspect_ratio;
  int image_height;
  int image_width;

  // Camera
  float viewport_height;
  float viewport_width;
  float focal_length;

  bmp::bmp_image* img = nullptr;
  camera* cam = nullptr;

  int AA_samples_per_pixel = 50;            // Anti Aliasing oversampling
  int diffuse_max_bounce_num = 10;          // Diffuse bounce number
  float diffuse_bounce_brightness = 0.5f;

  int parallel_chunks_num = 8;

public:
  frame_renderer(int width, int height, camera* cam);
  ~frame_renderer();

  color3 ray_color(const ray& r, const hittable_list& world, int depth);
  void render(const hittable_list& world);
  void save(const char* file_name);
};