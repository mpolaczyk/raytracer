// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"





int main()
{
  random_cache::init();
    
  vec3 look_from = vec3(0.9f, 1.7f, 2.0f);
  vec3 look_at = vec3(0.0f, 0.0f, 0.0f);
  float field_of_view = 70.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float aperture = 0.05f;
  float dist_to_focus = (look_from - look_at).length();

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
  std::vector<sphere> spheres;
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::red_diffuse_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::glass_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::green_diffuse_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::metal_matt_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::white_diffuse_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::glass_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::yellow_diffuse_preset));
  spheres.push_back(sphere(point3(0.0f, 0.0f, 0.0f), 0.4f, &material::metal_shiny_preset));
  for (int i = 0;i < spheres.size(); i++)
  {
    float alpha = i * (2*pi / spheres.size());
    spheres[i].center.e[0] = 1.1f * sin(alpha);
    spheres[i].center.e[2] = 1.1f * cos(alpha);
    world.add(spheres[i]);
  }
  for (int i = 0; i < spheres.size(); i++)
  {
    float alpha = (i * 2.0f * pi / spheres.size()) + 3*pi/8;
    spheres[i].center.e[0] = 1.8f * sin(alpha);
    spheres[i].center.e[2] = 1.8f * cos(alpha);
    world.add(spheres[i]);
  }
  world.add(sphere(point3(0.f, 0.0f, 0.f), 0.6f, &material::glass_preset));
  world.add(sphere(point3(0.f, -100.5f, -1.f), 100.f, &material::metal_matt_preset));

  frame_renderer renderer = frame_renderer(resolution_horizontal, resolution_vertical, renderer_settings::high_quality_preset);

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
