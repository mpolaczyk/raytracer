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
static inline const char* threading_strategy_names[] =
{
  "None",
  "PLL for each",
  "Thread poll"
};

struct renderer_config
{
  static renderer_config ten_thousand_per_pixel_preset;
  static renderer_config thousand_per_pixel_preset;
  static renderer_config super_mega_ultra_high_quality_preset;
  static renderer_config mega_ultra_high_quality_preset;
  static renderer_config ultra_high_quality_preset;
  static renderer_config high_quality_preset;
  static renderer_config medium_quality_preset;
  static renderer_config low_quality_preset;

  // Anti Aliasing oversampling
  int AA_samples_per_pixel = 20;            

  // Diffuse reflection
  int diffuse_max_bounce_num = 7;
  float diffuse_bounce_brightness = 0.6f;

  vec3 background = vec3(0.0f, 0.0f, 0.0f);

  // How work is split
  int chunks_num = 512;
  chunk_strategy_type chunks_strategy = chunk_strategy_type::rectangles;

  // How work is processed
  threading_strategy_type threading_strategy = threading_strategy_type::thread_pool;
  int threads_num = 0; // Apples only to thread pool strategy, 0 enforces std::thread::hardware_concurrency()
};

class frame_renderer
{
public:
  frame_renderer();
  ~frame_renderer();

  uint32_t image_height;
  uint32_t image_width;

  renderer_config settings;
  camera cam;

  bmp::bmp_image* img_bgr = nullptr;
  bmp::bmp_image* img_rgb = nullptr;

  void set_config(uint32_t width, uint32_t height, const renderer_config& in_settings);
  void render_multiple(const hittable_list& in_world, const std::vector<std::pair<uint32_t, camera_config>>& in_camera_states);
  void render_single(const hittable_list& in_world, const camera_config& in_camera_state, int frame_id = 0);

private:
  void render(const hittable_list& in_world);
  void render_chunk(const hittable_list& in_world, const chunk& in_chunk);
  vec3 ray_color(const ray& in_ray, const hittable_list& in_world, const vec3& in_background, uint32_t depth);
  void save(const char* file_name);
};