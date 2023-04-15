#include "stdafx.h"

#include "async_renderer_base.h"

#include <vector>
#include <ppl.h>

#include "gfx/bmp.h"
#include "math/hittables.h"
#include "math/materials.h"
#include "math/pdf.h"

async_renderer_base::async_renderer_base()
{
  worker_thread = std::thread(&async_renderer_base::async_job, this);
}
async_renderer_base::~async_renderer_base()
{
  job_state.requested_stop = true;
  worker_semaphore.release();
  worker_thread.join();
  if (job_state.img_rgb != nullptr) delete job_state.img_rgb;
  if (job_state.img_bgr != nullptr) delete job_state.img_bgr;
}

void async_renderer_base::set_config(const renderer_config& in_renderer_config, const scene& in_scene, const camera_config& in_camera_config)
{
  if (job_state.is_working) return;

  bool force_recreate_buffers = job_state.image_width != in_renderer_config.resolution_horizontal || job_state.image_height != in_renderer_config.resolution_vertical;

  // Copy all objects on purpose
  // - allows original scene to be edited while this one is rendering
  // - allows to detect if existing is dirty
  job_state.image_width = in_renderer_config.resolution_horizontal;
  job_state.image_height = in_renderer_config.resolution_vertical;
  job_state.renderer_conf = in_renderer_config;
  job_state.scene_root = *in_scene.clone();
  job_state.cam.set_camera(in_camera_config);

  // Delete buffers 
  if (job_state.img_rgb != nullptr)
  {
    if (force_recreate_buffers || !job_state.renderer_conf.reuse_buffer)
    {
      delete job_state.img_rgb;
      delete job_state.img_bgr;
      job_state.img_rgb = nullptr;
      job_state.img_bgr = nullptr;
    }
  }

  // Create new buffers if they are missing
  if (job_state.img_rgb == nullptr)
  {
    job_state.img_rgb = new bmp::bmp_image(job_state.image_width, job_state.image_height);
    job_state.img_bgr = new bmp::bmp_image(job_state.image_width, job_state.image_height);
  }

  std::cout << "Frame renderer: " << job_state.image_width << "x" << job_state.image_height << std::endl;
}

void async_renderer_base::render_single_async()
{
  if (job_state.is_working) return;
  
  job_state.is_working = true;
  worker_semaphore.release();
}

bool async_renderer_base::is_world_dirty(const scene& in_scene)
{
  return job_state.scene_root.get_type_hash() != in_scene.get_type_hash();
}

bool async_renderer_base::is_renderer_setting_dirty(const renderer_config& in_renderer_config)
{
  return job_state.renderer_conf.get_type_hash() != in_renderer_config.get_type_hash();
}

bool async_renderer_base::is_renderer_type_different(const renderer_config& in_renderer_config)
{
  return job_state.renderer_conf.type != in_renderer_config.type;
}

bool async_renderer_base::is_camera_setting_dirty(const camera_config& in_camera_config)
{
  return job_state.cam.get_type_hash() != in_camera_config.get_type_hash();
}

void async_renderer_base::async_job()
{
  while (true)
  {
    worker_semaphore.acquire();
    if(job_state.requested_stop) { break; }

    benchmark::instance benchmark_render;
    benchmark_render.start("Render");
    render();
    job_state.benchmark_render_time = benchmark_render.stop();

    if (save_output)
    {
      char image_file_name[100];
      std::sprintf(image_file_name, paths::get_render_output_file_path().c_str());

      benchmark::instance benchmark_save;
      benchmark_save.start("Save");
      save(image_file_name);
      job_state.benchmark_save_time = benchmark_save.stop();
    }

    job_state.is_working = false;
  }
}

void async_renderer_base::save(const char* file_name)
{
  job_state.img_bgr->save_to_file(file_name);
}