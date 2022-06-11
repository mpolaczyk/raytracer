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
  int resolution_horizontal = (int)((float)resolution_vertical) * aspect_ratio;

  std::vector<std::pair<uint32_t, camera_setup>> camera_states;
  camera_setup state0 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);
  camera_setup state1 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 1.0f);
  camera_setup state2 = camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, 0.0f);

  camera_states.push_back(std::make_pair(0, state0));
  camera_states.push_back(std::make_pair(10, state1));
  camera_states.push_back(std::make_pair(20, state2));

  sphere_list world;
  world.add(sphere(point3(-1.0f, 0.0f, -1.0f), 0.3f, &material::red_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, -1.0f), 0.3f, &material::green_diffuse_preset));
  world.add(sphere(point3(-1.0f, 0.0f, 1.0f), 0.3f, &material::yellow_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, 1.0f), 0.3f, &material::metal_shiny_preset));
  world.add(sphere(point3(0.f, 0.0f, 0.f), 0.5f, &material::glass_preset));
  world.add(sphere(point3(0.f, -100.5f, -1.f), 100.f, &material::metal_matt_preset));

  frame_renderer renderer = frame_renderer(resolution_horizontal, resolution_vertical, renderer_settings::medium_quality_preset);

  if (true)
  {
    renderer.render_single(world, state0);
  }
  else
  {
    renderer.render_multiple(world, camera_states);
  }
  
  return 0;
}
