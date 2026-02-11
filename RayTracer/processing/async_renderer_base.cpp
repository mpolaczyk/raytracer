#include "stdafx.h"

#include <thread>
#include <semaphore>

#include "async_renderer_base.h"

#include "gfx/bmp.h"
#include "math/hittables.h"
#include "math/materials.h"
#include "math/pdf.h"
#include "math/camera.h"
#include "processing/benchmark.h"

async_renderer_base::async_renderer_base()
{
  worker_thread = new std::thread(&async_renderer_base::async_job, this);
  worker_semaphore = new std::binary_semaphore(0);
}
async_renderer_base::~async_renderer_base()
{
  job_state.requested_stop = true;
  worker_semaphore->release();
  worker_thread->join();
  delete worker_semaphore;
  delete worker_thread;
  delete job_state.scene_root;
  delete job_state.cam;
  if (job_state.img_rgb != nullptr) delete job_state.img_rgb;
  if (job_state.img_bgr != nullptr) delete job_state.img_bgr;
}

void async_renderer_base::set_config(const renderer_config* in_renderer_config, const scene* in_scene, const camera_config* in_camera_config)
{
  assert(in_scene != nullptr);
  assert(in_camera_config != nullptr);
  assert(in_renderer_config != nullptr);

  if (job_state.is_working) return;

  bool force_recreate_buffers = job_state.image_width != in_renderer_config->resolution_horizontal || job_state.image_height != in_renderer_config->resolution_vertical;

  // Copy all objects on purpose
  // - allows original scene to be edited while this one is rendering
  // - allows to detect if existing is dirty
  job_state.image_width = in_renderer_config->resolution_horizontal;
  job_state.image_height = in_renderer_config->resolution_vertical;
  if (job_state.renderer_conf == nullptr)
  {
    job_state.renderer_conf = new renderer_config();
  }
  *job_state.renderer_conf = *in_renderer_config;
  job_state.scene_root = in_scene->clone();
  if (job_state.cam == nullptr)
  {
    job_state.cam = new camera();
  }
  job_state.cam->configure(in_camera_config);

  // Delete buffers 
  if (job_state.img_rgb != nullptr)
  {
    if (force_recreate_buffers || !job_state.renderer_conf->reuse_buffer)
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

  logger::info("Frame renderer: {0}x{1}", job_state.image_width, job_state.image_height);
}

void async_renderer_base::render_single_async()
{
  if (job_state.is_working) return;
  
  job_state.is_working = true;
  worker_semaphore->release();
}

void async_renderer_base::render_single_sync()
{
  if (job_state.is_working) return;

  job_state.is_working = true;
  run_render_job();
}

bool async_renderer_base::is_world_dirty(const scene* in_scene)
{
  assert(in_scene != nullptr);
  assert(job_state.scene_root != nullptr);
  return job_state.scene_root->get_hash() != in_scene->get_hash();
}

bool async_renderer_base::is_renderer_setting_dirty(const renderer_config* in_renderer_config)
{
  assert(in_renderer_config != nullptr);
  if (job_state.renderer_conf == nullptr)
  {
    return true;
  }
  return job_state.renderer_conf->get_hash() != in_renderer_config->get_hash();
}

bool async_renderer_base::is_renderer_type_different(const renderer_config* in_renderer_config)
{
  assert(in_renderer_config != nullptr);
  if (job_state.renderer_conf == nullptr)
  {
    return true;
  }
  return job_state.renderer_conf->type != in_renderer_config->type;
}

bool async_renderer_base::is_camera_setting_dirty(const camera_config* in_camera_config)
{
  assert(in_camera_config != nullptr);
  assert(job_state.cam != nullptr);
  return job_state.cam->get_hash() != in_camera_config->get_hash();
}

void async_renderer_base::async_job()
{
  while (true)
  {
    worker_semaphore->acquire();
    if(job_state.requested_stop) { break; }

    run_render_job();
  }
}

void async_renderer_base::run_render_job()
{
  benchmark::instance benchmark_render;
  benchmark_render.start("Render");
  render();
  job_state.benchmark_render_time = benchmark_render.stop();

  if (save_output)
  {
    char image_file_name[300];  // Run-Time Check Failure #2 - Stack around the variable 'image_file_name' was corrupted.
    std::sprintf(image_file_name, io::get_render_output_file_path().c_str());

    benchmark::instance benchmark_save;
    benchmark_save.start("Save");
    save(image_file_name);
    job_state.benchmark_save_time = benchmark_save.stop();
  }

  job_state.is_working = false;
}

void async_renderer_base::save(const char* file_name)
{
  job_state.img_bgr->save_to_file(file_name);
}