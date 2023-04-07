#include "stdafx.h"

#include "async_renderer_base.h"

#include <vector>
#include <ppl.h>

#include "gfx/bmp.h"
#include "thread_pool.h"
#include "math/hittables.h"
#include "math/materials.h"
#include "math/pdf.h"

async_renderer_base::async_renderer_base()
{
  worker_thread = std::thread(&async_renderer_base::async_job, this);
}
async_renderer_base::~async_renderer_base()
{
  ajs.requested_stop = true;
  worker_semaphore.release();
  worker_thread.join();
  if (ajs.img_rgb != nullptr) delete ajs.img_rgb;
  if (ajs.img_bgr != nullptr) delete ajs.img_bgr;
}

void async_renderer_base::set_config(const renderer_config& in_settings, const scene& in_scene, const camera_config& in_camera_state)
{
  if (ajs.is_working) return;

  bool force_recreate_buffers = ajs.image_width != in_settings.resolution_horizontal || ajs.image_height != in_settings.resolution_vertical;

  // Copy all objects on purpose
  // - allows original scene to be edited while this one is rendering
  // - allows to detect if existing is dirty
  ajs.image_width = in_settings.resolution_horizontal;
  ajs.image_height = in_settings.resolution_vertical;
  ajs.settings = in_settings;
  ajs.scene_root = *in_scene.clone();
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
    ajs.img_rgb = new bmp::bmp_image(ajs.image_width, ajs.image_height);
    ajs.img_bgr = new bmp::bmp_image(ajs.image_width, ajs.image_height);
  }

  std::cout << "Frame renderer: " << ajs.image_width << "x" << ajs.image_height << std::endl;
}

void async_renderer_base::render_single_async()
{
  if (ajs.is_working) return;
  
  ajs.is_working = true;
  worker_semaphore.release();
}

bool async_renderer_base::is_world_dirty(const scene& in_scene)
{
  return ajs.scene_root.get_type_hash() != in_scene.get_type_hash();
}

bool async_renderer_base::is_renderer_setting_dirty(const renderer_config& in_settings)
{
  return ajs.settings.get_type_hash() != in_settings.get_type_hash();
}

bool async_renderer_base::is_camera_setting_dirty(const camera_config& in_camera_state)
{
  return ajs.cam.get_type_hash() != in_camera_state.get_type_hash();
}

void async_renderer_base::async_job()
{
  while (true)
  {
    worker_semaphore.acquire();
    if(ajs.requested_stop) { break; }

    benchmark::instance benchmark_render;
    benchmark_render.start("Render");
    render();
    ajs.benchmark_render_time = benchmark_render.stop();

    char image_file_name[100];
    std::sprintf(image_file_name, paths::get_render_output_file_path().c_str());

    benchmark::instance benchmark_save;
    benchmark_save.start("Save");
    save(image_file_name);
    ajs.benchmark_save_time = benchmark_save.stop();

    ajs.is_working = false;
  }
}

void async_renderer_base::save(const char* file_name)
{
  ajs.img_bgr->save_to_file(file_name);
}