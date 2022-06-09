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
  
  vec3 view_from = vec3(0.0f, 0.0f, 1.0f);
  vec3 view_at = vec3(0.0f, 0.0f, 0.0f);
  float dist_to_focus = (view_from - view_at).length();

  float field_of_view = 90.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float aperture = 0.0f;
  float type = 1.0f;

  camera cam(view_from, view_at, field_of_view, aspect_ratio, aperture, dist_to_focus, type);

  int resolution_vertical = 1080;
  frame_renderer renderer = frame_renderer(
    (int)((float)resolution_vertical * aspect_ratio), 
    resolution_vertical, 
    renderer_settings::low_quality_preset, 
    cam);
   
  material white_diffuse;
  white_diffuse.type = material_type::diffuse;
  white_diffuse.albedo = white_blue;

  material green_diffuse;
  green_diffuse.albedo = green;
  green_diffuse.type = material_type::diffuse;

  material yellow_diffuse;
  yellow_diffuse.albedo = yellow;
  yellow_diffuse.type = material_type::diffuse;

  material red_diffuse;
  red_diffuse.albedo = red;
  red_diffuse.type = material_type::diffuse;

  material metal_shiny;
  metal_shiny.albedo = yellow;
  metal_shiny.type = material_type::metal_shiny;

  material dialectric;
  dialectric.albedo = grey;
  dialectric.type = material_type::dialectric;

  material metal_matt;
  metal_matt.albedo = grey;
  metal_matt.type = material_type::metal_matt;

  sphere_list world;
  world.add(sphere(point3(0.0f, 0.0f, 0.0f), 0.3f, &red_diffuse));
  world.add(sphere(point3(1.0f, 0.0f, -1.0f), 0.3f, &green_diffuse));
  world.add(sphere(point3(2.0f, 0.0f, -2.0f), 0.3f, &yellow_diffuse));

  //world.add(sphere(point3(3.f, 0.f, -3.f), 0.3f, &dialectric));
  world.add(sphere(point3(0.f, -100.5f, -1.f), 100.f, &metal_matt));

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