#pragma once

#include <thread>
#include <semaphore>

#include "vec3.h"
#include "hittables.h"
#include "ray.h"
#include "chunk_generator.h"
#include "camera.h"
#include "bmp.h"


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

  bool allow_emissive = true;
};

class frame_renderer
{
public:
  frame_renderer();
  ~frame_renderer();

  // Only allowed when worker thread is not running
  void set_config(uint32_t width, uint32_t height, const renderer_config& in_settings, const hittable_list& in_world, const camera_config& in_camera_state);
  void render_single_async();

  // Allowed when worker thread is running
  bool is_working() const { return ajs.is_working; }
  uint64_t get_render_time() const { return ajs.benchmark_render_time; }
  uint64_t get_save_time() const { return ajs.benchmark_save_time; }
  uint8_t* get_img_bgr() { return ajs.img_bgr->get_buffer(); }
  uint8_t* get_img_rgb() { return ajs.img_rgb->get_buffer(); }

  //void render_multiple(const hittable_list& in_world, const std::vector<std::pair<uint32_t, camera_config>>& in_camera_states);
  //void render_single(const hittable_list& in_world, const camera_config& in_camera_state, int frame_id = 0);

private:
  void render();
  void render_chunk(const chunk& in_chunk);
  vec3 ray_color(const ray& in_ray, const vec3& in_background, uint32_t depth);
  void save(const char* file_name);

  // Synchronization - fire and forget
  // No job cancellation
  std::thread worker_thread;
  std::binary_semaphore worker_semaphore{ 0 };
  void async_job();

  // Data assess pattern for async job state
  // - RW for job thread
  // - R for main thread while processing, only through const getters
  // - W for game thread only using set_config
  struct 
  {
    bool is_working = false;

    uint32_t image_height = 0;
    uint32_t image_width = 0;

    renderer_config settings;
    camera cam;
    hittable_list world;
    
    bmp::bmp_image* img_bgr = nullptr;
    bmp::bmp_image* img_rgb = nullptr;

    uint64_t benchmark_render_time = 0;
    uint64_t benchmark_save_time = 0;
  } ajs; 
};