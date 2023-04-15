#pragma once

#include <thread>
#include <semaphore>

#include "math/vec3.h"
#include "math/hittables.h"
#include "math/ray.h"
#include "chunk_generator.h"
#include "math/camera.h"
#include "gfx/bmp.h"

#include "app/json/serializable.h"
#include "app/factories.h"

namespace bmp
{
  struct bmp_image;
}
class camera;

class renderer_config : serializable<nlohmann::json>
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

  nlohmann::json serialize();
  void deserialize(const nlohmann::json& j);

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(renderer_config, rays_per_pixel, ray_bounces, type, reuse_buffer, resolution_vertical, resolution_horizontal); // to_json only

  inline uint32_t get_type_hash() const
  {
    return hash_combine(rays_per_pixel, ray_bounces, (int)type, (int)type);
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
  void set_config(const renderer_config& in_renderer_config, const scene& in_scene, const camera_config& in_camera_config);
  // 2. Request work
  void render_single_async();
  
  // State checks
  bool is_world_dirty(const scene& in_scene);
  bool is_renderer_setting_dirty(const renderer_config& in_renderer_config);
  bool is_renderer_type_different(const renderer_config& in_renderer_config);
  bool is_camera_setting_dirty(const camera_config& in_camera_config);
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

    uint32_t image_height = 0;
    uint32_t image_width = 0;

    renderer_config renderer_conf;
    camera cam;
    scene scene_root;
    
    bmp::bmp_image* img_bgr = nullptr;
    bmp::bmp_image* img_rgb = nullptr;

    uint64_t benchmark_render_time = 0;
    uint64_t benchmark_save_time = 0;
  } job_state; 

private:
  // Synchronization - fire and forget
  // No job cancellation
  std::thread worker_thread;
  std::binary_semaphore worker_semaphore{ 0 };
  void async_job();
  void save(const char* file_name);
};