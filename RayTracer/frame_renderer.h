#pragma once

#include "vec3.h"
#include "sphere.h"
#include "ray.h"
#include "chunk_generator.h"
#include "camera.h"

namespace bmp
{
  struct bmp_image;
}
class camera;

enum class threading_strategy_type
{
  none = 0,
  pll_for_each,
  thread_pool
};

struct renderer_settings
{
  static renderer_settings high_quality_preset;
  static renderer_settings medium_quality_preset;
  static renderer_settings low_quality_preset;

  // Anti Aliasing oversampling
  uint32_t AA_samples_per_pixel = 20;            

  // Diffuse reflection
  uint32_t diffuse_max_bounce_num = 7;
  float diffuse_bounce_brightness = 0.6f;

  // How work is split
  uint32_t chunks_num = 512;
  chunk_strategy_type chunks_strategy = chunk_strategy_type::rectangles;

  // How work is processed
  threading_strategy_type threading_strategy = threading_strategy_type::thread_pool;
  uint32_t threads_num = 0; // Apples only to thread pool strategy, 0 enforces std::thread::hardware_concurrency()
};

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

  const renderer_settings settings;
  const camera cam;

  bmp::bmp_image* img = nullptr;
  
public:
  frame_renderer(uint32_t width, uint32_t height, const renderer_settings& settings, const camera& cam);
  ~frame_renderer();

  color3 ray_color(const ray& r, const sphere_list& world, uint32_t depth);
  void render(const sphere_list& world);
  void render_chunk(const sphere_list& world, const chunk& ch);
  void save(const char* file_name);
};