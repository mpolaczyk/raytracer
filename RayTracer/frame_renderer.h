#pragma once
#include <vector>
#include "vec3.h"
#include "hittable.h"
#include "ray.h"
#include "bmp.h"

class camera;
using namespace bmp;

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

  bmp_image* img = nullptr;
  camera* cam = nullptr;

  int samples_per_pixel = 100;
  std::vector<float> random_floats;

public:
  frame_renderer(int width, int height, camera* cam);
  ~frame_renderer();

  color3 ray_color(const ray& r, const hittable_list& world, int depth);
  void render(const hittable_list& world_list);
  void save(const char* file_name);
};