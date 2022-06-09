// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

int main()
{
  random_cache::init();

  const char* image_file_name = (const char*)"image.bmp";
  
  vec3 look_from = vec3(0.0f, 1.0f, 2.0f);
  vec3 look_at = vec3(0.0f, 0.0f, 0.0f);
  float field_of_view = 80.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float aperture = 0.1f;
  float dist_to_focus = (look_from - look_at).length();
  float type = 0.0f;

  camera cam;
  cam.set_camera(camera_setup(look_from, look_at, field_of_view, aspect_ratio, aperture, dist_to_focus, type));

  int resolution_vertical = 1080;
  frame_renderer renderer = frame_renderer(
    (int)((float)resolution_vertical * aspect_ratio), 
    resolution_vertical, 
    renderer_settings::medium_quality_preset, 
    cam);
   
  sphere_list world;
  world.add(sphere(point3(-1.0f, 0.0f, -1.0f), 0.3f, &material::red_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, -1.0f), 0.3f, &material::green_diffuse_preset));
  world.add(sphere(point3(-1.0f, 0.0f, 1.0f), 0.3f, &material::yellow_diffuse_preset));
  world.add(sphere(point3(1.0f, 0.0f, 1.0f), 0.3f, &material::metal_shiny_preset));

  world.add(sphere(point3(0.f, 0.0f, 0.f), 0.5f, &material::glass_preset));
  world.add(sphere(point3(0.f, -100.5f, -1.f), 100.f, &material::metal_matt_preset));

  {
    benchmark::scope_counter benchmark_render("Render");
    renderer.render(world);
  }

  {
    benchmark::scope_counter benchmark_render("Save");
    renderer.save(image_file_name);
  }

  return 0;
}