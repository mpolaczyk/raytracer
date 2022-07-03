#include "stdafx.h"

#include "frame_renderer.h"

#include <vector>
#include <ppl.h>

#include "bmp.h"
#include "thread_pool.h"
#include "hittables.h"
#include "materials.h"

frame_renderer::frame_renderer()
{
  worker_thread = std::thread(&frame_renderer::async_job, this);
}
frame_renderer::~frame_renderer()
{
  worker_thread.join();
  if (ajs.img_rgb != nullptr) delete ajs.img_rgb;
  if (ajs.img_bgr != nullptr) delete ajs.img_bgr;
}

void frame_renderer::set_config(uint32_t width, uint32_t height, const renderer_config& in_settings, const hittable_list& in_world, const camera_config& in_camera_state)
{
  if (ajs.is_working) return;

  bool force_recreate_buffers = ajs.image_width != width || ajs.image_height != height;

  // Copy all objects on purpose
  // - allows original scene to be edited while this one is rendering
  // - allows to detect if existing is dirty
  ajs.image_width = width;
  ajs.image_height = height;
  ajs.settings = in_settings;
  ajs.world = *in_world.clone();
  ajs.cam.set_camera(in_camera_state);

  // Delete buffers 
  if (ajs.img_rgb != nullptr)
  {
    if (force_recreate_buffers || !ajs.settings.reuse_buffer)
    {
      delete ajs.img_rgb;
      delete ajs.img_bgr;
      ajs.img_rgb = nullptr;
      ajs.img_bgr = nullptr;
    }
  }

  // Create new buffers if they are missing
  if (ajs.img_rgb == nullptr)
  {
    ajs.img_rgb = new bmp::bmp_image(width, height);
    ajs.img_bgr = new bmp::bmp_image(width, height);
  }

  std::cout << "Frame renderer: " << width << "x" << height << std::endl;
}

void frame_renderer::render_single_async()
{
  if (ajs.is_working) return;
  
  ajs.is_working = true;
  worker_semaphore.release();
}

//void frame_renderer::render_multiple(const hittable_list& in_world, const std::vector<std::pair<uint32_t, camera_config>>& in_camera_states)
//{
//  camera cam;
//  if (in_camera_states.size() < 2)
//  {
//    std::cout << "More than two camera states required" << std::endl;
//    return;
//  }
//  for (int setup_id = 0; setup_id < in_camera_states.size() - 1; setup_id++)
//  {
//    int frame_begin = in_camera_states[setup_id].first;
//    int frame_end = in_camera_states[setup_id + 1].first;
//    camera_config setup_begin = in_camera_states[setup_id].second;
//    camera_config setup_end = in_camera_states[setup_id + 1].second;
//
//    for (int frame_id = frame_begin; frame_id < frame_end; frame_id++)
//    {
//      char name[100];
//      std::sprintf(name, "setup_id=%d frame_id=%d", setup_id, frame_id);
//      std::cout << name << std::endl;
//
//      float f = (float)(frame_id - frame_begin) / (float)(frame_end - frame_begin);
//      render_single(in_world, camera_config::lerp(setup_begin, setup_end, f), frame_id);
//    }
//  }
//}
//
//void frame_renderer::render_single(const hittable_list& in_world, const camera_config& in_camera_state, int frame_id)
//{
//  cam.set_camera(in_camera_state);
//  {
//    benchmark::instance benchmark_render;
//    benchmark_render.start("Render");
//
//    render(in_world);
//
//    benchmark_render_time = benchmark_render.stop();
//  }
//
//  char image_file_name[100];
//  std::sprintf(image_file_name, "image_%d.bmp", frame_id);
//  {
//    benchmark::instance benchmark_save;
//    benchmark_save.start("Save");
//
//    save(image_file_name);
//
//    benchmark_save_time = benchmark_save.stop();
//  }
//}

bool frame_renderer::is_world_dirty(const hittable_list& in_world)
{
  return ajs.world.get_type_hash() != in_world.get_type_hash();
}

bool frame_renderer::is_renderer_setting_dirty(const renderer_config& in_settings)
{
  return ajs.settings.get_type_hash() != in_settings.get_type_hash();
}

bool frame_renderer::is_camera_setting_dirty(const camera_config& in_camera_state)
{
  return ajs.cam.get_type_hash() != in_camera_state.get_type_hash();
}

void frame_renderer::async_job()
{
  while (true)
  {
    worker_semaphore.acquire();

    benchmark::instance benchmark_render;
    benchmark_render.start("Render");
    render();
    ajs.benchmark_render_time = benchmark_render.stop();

    char image_file_name[100];
    std::sprintf(image_file_name, "last_render.bmp");

    benchmark::instance benchmark_save;
    benchmark_save.start("Save");
    save(image_file_name);
    ajs.benchmark_save_time = benchmark_save.stop();

    ajs.is_working = false;
  }
}

void frame_renderer::render()
{
  // Build chunks of work
  std::vector<chunk> chunks;
  chunk_generator::generate_chunks(ajs.settings.chunks_strategy, ajs.settings.chunks_num, ajs.image_width, ajs.image_height, chunks);

  if (ajs.settings.shuffle_chunks)
  {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(chunks.begin(), chunks.end(), g);
  }

  //for (const auto& ch : chunks)
  //{
  //  std::cout << "Chunk=" << ch.id << " x=" << ch.x << " y=" << ch.y << " size_x=" << ch.size_x << " size_y=" << ch.size_y << std::endl;
  //}
  
  // Process chunks on parallel
  if (ajs.settings.threading_strategy == threading_strategy_type::none)
  {
    chunk ch;
    ch.id = 1;
    ch.size_x = ajs.image_width;
    ch.size_y = ajs.image_height;
    render_chunk(ch);
  }
  if (ajs.settings.threading_strategy == threading_strategy_type::thread_pool)
  {
    thread_pool pool;
    for (const chunk& ch : chunks)
    {
      pool.queue_job([&]() { render_chunk(ch); });
    }
    pool.start(ajs.settings.threads_num > 0 ? ajs.settings.threads_num : std::thread::hardware_concurrency());
    while (pool.is_busy()) { }
    pool.stop();
  }
  else if (ajs.settings.threading_strategy == threading_strategy_type::pll_for_each)
  {
    concurrency::parallel_for_each(begin(chunks), end(chunks), [&](chunk ch) { render_chunk(ch); });
  }  
}

void frame_renderer::render_chunk(const chunk& in_chunk)
{
  std::thread::id thread_id = std::this_thread::get_id();

  std::ostringstream oss;
  oss << "Thread=" << thread_id << " Chunk=" << in_chunk.id;
  const char* name = oss.str().c_str();
  benchmark::scope_counter benchmark_render_chunk(name, false);

  for (int y = in_chunk.y; y < in_chunk.y + in_chunk.size_y; ++y)
  {
    for (int x = in_chunk.x; x < in_chunk.x + in_chunk.size_x; ++x)
    {
      benchmark::instance timer;
      if (ajs.settings.pixel_time_coloring)
      {
        timer.start("RayColor", false);
      }
      // Anti Aliasing done at the ray level, multiple rays for each pixel.
      vec3 pixel_color;
      for (int c = 0; c < ajs.settings.AA_samples_per_pixel; c++)
      {
        float u = (float(x) + random_cache::get_float()) / (ajs.image_width - 1);
        float v = (float(y) + random_cache::get_float()) / (ajs.image_height - 1);
        ray r = ajs.cam.get_ray(u, v);
        pixel_color += ray_color(r, ajs.settings.background, ajs.settings.diffuse_max_bounce_num);
      }
      // Save to bmp
      bmp::bmp_pixel p;
      if (ajs.settings.pixel_time_coloring)
      {
        uint64_t t = timer.stop();
        p = bmp::bmp_pixel(t / (float)ajs.settings.AA_samples_per_pixel * ajs.settings.pixel_time_coloring_scale);
      }
      else
      {
        p = bmp::bmp_pixel(pixel_color / (float)ajs.settings.AA_samples_per_pixel);
      }
      ajs.img_bgr->draw_pixel(x, y, &p);
      ajs.img_rgb->draw_pixel(x, y, &p, bmp::bmp_format::rgba);
    }
  }
}

vec3 inline frame_renderer::ray_color(const ray& in_ray, const vec3& in_background, uint32_t depth)
{
  if (depth <= 0)
  {
    return c_black;
  }

  hit_record hit;
  if (ajs.world.hit(in_ray, 0.001f, infinity, hit))
  {
    ray scattered;
    vec3 attenuation;
    vec3 emitted;
    if (ajs.settings.allow_emissive)
    {
      emitted = hit.material_ptr->emitted(hit.u, hit.v, hit.p);
    }
    if (hit.material_ptr->scatter(in_ray, hit, attenuation, scattered))
    {
      return emitted+attenuation * ray_color(scattered, in_background, depth - 1);
    }
    
    if (ajs.settings.allow_emissive)
    {
      return emitted;
    }
  }
  if (ajs.settings.allow_emissive)
  {
    return c_black;
  }
  return in_background; // source of light for non emissive mode

  // OLD CODE
  //hit_record hit;
  //if (in_world.hit(in_ray, 0.001f, infinity, hit))  // 0.001f to fix "shadow acne"
  //{
  //  ray scattered;
  //  vec3 attenuation;
  //  if (hit.mat->scatter(in_ray, hit, attenuation, scattered))
  //  {
  //    return attenuation * ray_color(scattered, in_world, in_background, depth - 1);
  //  }
  //  return black;
  //  
  //}
  //
  //vec3 unit_direction = in_ray.direction;              // unit_vector(r.direction)
  //float y =  0.5f * (unit_direction.y + 1.0f);  // base blend based on y component of a ray
  //return (1.0f - y) * white + y * white_blue;     // linear blend (lerp) between white and blue
}

void frame_renderer::save(const char* file_name)
{
  ajs.img_bgr->save_to_file(file_name);
}