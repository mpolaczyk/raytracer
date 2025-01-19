#pragma once

#include "math/vec3.h"
#include "math/hittables.h"
#include "math/ray.h"
#include "chunk_generator.h"
#include "gfx/bmp.h"

#include "app/factories.h"

namespace std
{
  class thread;
  typedef ptrdiff_t;
  template <ptrdiff_t _Least_max_value>
  class counting_semaphore;
  using binary_semaphore = counting_semaphore<1>;
}
namespace bmp
{
  struct bmp_image;
}

class camera;
class camera_config;
class scene;

class renderer_config
{
public:
  // Anti Aliasing oversampling
  int rays_per_pixel = 20;

  // Diffuse reflection
  int ray_bounces = 7;

  // How work is processed
  renderer_type type = renderer_type::reference;
    
  // Draw in the same memory - real time update
  bool reuse_buffer = true;

  int resolution_vertical = 0;
  int resolution_horizontal = 0;

  // Manually set the brightest point of an image used for tone mapping
  float white_point = 1.0f;

  inline uint32_t get_hash() const
  {
    return hash::combine(rays_per_pixel, ray_bounces, (int)type, hash::get(white_point));
  }
};

class async_renderer_base
{
public:
  async_renderer_base();
  ~async_renderer_base();

  // Renderer instance interface
  virtual std::string get_name() const = 0;
  virtual void render() = 0;

  // Renderer public interface. Usage:
  // 1. Set scene, camera and settings first
  void set_config(const renderer_config* in_renderer_config, const scene* in_scene, const camera_config* in_camera_config);
  // 2. Request work
  void render_single_async();
  
  // State checks
  bool is_world_dirty(const scene* in_scene);
  bool is_renderer_setting_dirty(const renderer_config* in_renderer_config);
  bool is_renderer_type_different(const renderer_config* in_renderer_config);
  bool is_camera_setting_dirty(const camera_config* in_camera_config);
  bool is_working() const { return job_state.is_working; }

  uint64_t get_render_time() const { return job_state.benchmark_render_time; }
  uint64_t get_save_time() const { return job_state.benchmark_save_time; }
  uint8_t* get_img_bgr() { return job_state.img_bgr->get_buffer(); }
  uint8_t* get_img_rgb() { return job_state.img_rgb->get_buffer(); }

  bool save_output = true;

protected:
  // Data assess pattern for async job state
  // - RW for job thread
  // - R for main thread while processing, only through const getters
  // - W for game thread only using set_config
  struct 
  {
    bool is_working = false;
    bool requested_stop = false;

    int image_height = 0;
    int image_width = 0;

    renderer_config* renderer_conf = nullptr;
    camera* cam = nullptr;
    scene* scene_root = nullptr;
    
    bmp::bmp_image* img_bgr = nullptr;
    bmp::bmp_image* img_rgb = nullptr;

    uint64_t benchmark_render_time = 0;
    uint64_t benchmark_save_time = 0;
  } job_state; 

private:
  // Synchronization - fire and forget
  // No job cancellation
  std::thread* worker_thread = nullptr;
  std::binary_semaphore* worker_semaphore = nullptr;
  void async_job();
  void save(const char* file_name);
};