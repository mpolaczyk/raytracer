// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"


int main()
{
  random_cache::init();
    
  vec3 look_from = vec3(0.0f, 1.0f, 2.0f);
  vec3 look_at = vec3(0.0f, 0.0f, 0.0f);
  float field_of_view = 80.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float aperture = 0.1f;
  float dist_to_focus = (look_from - look_at).length();
  float type = 0.0f;

  int resolution_vertical = 1080;

  std::vector<std::pair<uint32_t, camera_setup>> camera_states;
  camera_setup state0 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
  camera_setup state1 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 1.0f);
  camera_setup state2 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
  camera_setup state3 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus+2.0f, 0.0f);
  camera_setup state4 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
  camera_setup state5 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus-1.0f, 0.0f);
  camera_setup state6 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);

  camera_states.push_back(std::make_pair(0, state0));
  camera_states.push_back(std::make_pair(10, state1));
  camera_states.push_back(std::make_pair(20, state2));
  camera_states.push_back(std::make_pair(30, state3));
  camera_states.push_back(std::make_pair(40, state4));
  camera_states.push_back(std::make_pair(50, state5));
  camera_states.push_back(std::make_pair(60, state6));

  sphere_list world;
  world.add(sphere(point3(-1.0f, 0.0f, -1.0f), 0.3f, &material::red_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, -1.0f), 0.3f, &material::green_diffuse_preset));
  world.add(sphere(point3(-1.0f, 0.0f, 1.0f), 0.3f, &material::yellow_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, 1.0f), 0.3f, &material::metal_shiny_preset));
  world.add(sphere(point3(0.f, 0.0f, 0.f), 0.5f, &material::glass_preset));
  world.add(sphere(point3(0.f, -100.5f, -1.f), 100.f, &material::metal_matt_preset));

  camera cam;
  
  for (int setup_id = 0; setup_id < camera_states.size() - 1; setup_id++)
  {
    int frame_begin = camera_states[setup_id].first;
    int frame_end = camera_states[setup_id + 1].first;
    camera_setup setup_begin = camera_states[setup_id].second;
    camera_setup setup_end = camera_states[setup_id + 1].second;

    for (int frame_id = frame_begin; frame_id < frame_end; frame_id++)
    {
      char name[100];
      std::sprintf(name, "setup_id=%d frame_id=%d", setup_id, frame_id);
      std::cout << name << std::endl;

      float f = (float)(frame_id - frame_begin) / (float)(frame_end - frame_begin);
      cam.set_camera(camera_setup::lerp(setup_begin, setup_end, f));

      frame_renderer renderer = frame_renderer(
        (int)((float)resolution_vertical * aspect_ratio),
        resolution_vertical,
        renderer_settings::medium_quality_preset,
        cam);

      {
        benchmark::scope_counter benchmark_render("Render");
        renderer.render(world);
      }

      char image_file_name[100];
      std::sprintf(image_file_name, "image_%d.bmp", frame_id);
      {
        benchmark::scope_counter benchmark_render("Save");
        renderer.save(image_file_name);
      }
    }
  }
  return 0;
}