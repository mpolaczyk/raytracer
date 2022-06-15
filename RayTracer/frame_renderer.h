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
  static renderer_settings ten_thousand_per_pixel_preset;
  static renderer_settings thousand_per_pixel_preset;
  static renderer_settings super_mega_ultra_high_quality_preset;
  static renderer_settings mega_ultra_high_quality_preset;
  static renderer_settings ultra_high_quality_preset;
  static renderer_settings high_quality_preset;
  static renderer_settings medium_quality_preset;
  static renderer_settings low_quality_preset;

  // Anti Aliasing oversampling
  uint32_t AA_samples_per_pixel = 20;            

  // Diffuse reflection
  uint32_t diffuse_max_bounce_num = 7;
  float diffuse_bounce_brightness = 0.6f;

  vec3 background = vec3(0.0f, 0.0f, 0.0f);

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
  camera cam;

  bmp::bmp_image* img = nullptr;
  
public:
  frame_renderer(uint32_t width, uint32_t height, const renderer_settings& in_settings);
  ~frame_renderer();

  void set_camera(const camera& in_cam);
  void render_multiple(const sphere_list& in_world, const std::vector<std::pair<uint32_t, camera_setup>>& in_camera_states);
  void render_single(const sphere_list& in_world, const camera_setup& in_camera_state, int frame_id = 0);

private:
  void render(const sphere_list& in_world);
  void render_chunk(const sphere_list& in_world, const chunk& in_chunk);
  vec3 ray_color(const ray& in_ray, const sphere_list& in_world, const vec3& in_background, uint32_t depth);
  void save(const char* file_name);
};