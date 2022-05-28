// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "stdafx.h"

#include "frame_renderer.h"
#include "hittable.h"
#include "camera.h"

int main()
{
  const char* image_file_name = (const char*)"image.bmp";
  float aspect_ratio = 16.0f / 9.0f;
  float focal_length = 1.0f;
  int resolution_vertical = 1440;

  camera cam(aspect_ratio, focal_length);
  frame_renderer renderer = frame_renderer((int)((float)resolution_vertical * aspect_ratio), resolution_vertical, &cam);
   
  hittable_list world;
  world.add(hittable(point3(0.f, 0.f, -1.f), 0.5f));
  world.add(hittable(point3(1.f, -0.2f, -1.f), 0.2f));
  world.add(hittable(point3(-1.f, -0.2f, -1.f), 0.2f));
  world.add(hittable(point3(0.f, -100.5f, -1.f), 100.f));

  {
    benchmark::scope_counter benchmark_render("Render");
    renderer.render(world);
  }

  benchmark::static_start("Save");
  renderer.save(image_file_name);
  benchmark::static_stop();

  return 0;
}