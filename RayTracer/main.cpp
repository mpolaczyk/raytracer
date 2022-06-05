// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

int main()
{
  const char* image_file_name = (const char*)"image.bmp";
  float aspect_ratio = 16.0f / 9.0f;
  float focal_length = 1.0f;
  int resolution_vertical = 1080;

  camera cam(aspect_ratio, focal_length, point3(0.0f, 0.0f, -0.2f));
  frame_renderer renderer = frame_renderer(
    (int)((float)resolution_vertical * aspect_ratio), 
    resolution_vertical, 
    renderer_settings::medium_quality_preset, 
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
  world.add(sphere(point3(0.8f, -0.2f, -1.4f), 0.2f, &red_diffuse));
  world.add(sphere(point3(-0.8f, -0.2f, -1.4f), 0.2f, &green_diffuse));
  world.add(sphere(point3(0.0f, 0.f, -6.0f), 0.4f, &yellow_diffuse));

  world.add(sphere(point3(0.f, 0.f, -1.f), 0.5f, &dialectric));
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