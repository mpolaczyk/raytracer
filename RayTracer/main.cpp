// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <memory>

#include "common.h"
#include "benchmark.h"
#include "frame_renderer.h"
#include "hittable.h"
#include "sphere.h"
#include "camera.h"

using std::make_shared;

int main()
{
  const char* image_file_name = (const char*)"image.bmp";
  float aspect_ratio = 16.0f / 9.0f;
  float focal_length = 1.0f;
  int resolution_vertical = 720;

  camera cam(aspect_ratio, focal_length);
  frame_renderer renderer = frame_renderer((int)((float)resolution_vertical * aspect_ratio), resolution_vertical, &cam);
   
  hittable_list world;
  world.add(make_shared<sphere>(point3(0.f, 0.f, -1.f), 0.5f));
  world.add(make_shared<sphere>(point3(1.f, -0.2f, -1.f), 0.2f));
  world.add(make_shared<sphere>(point3(-1.f, -0.2f, -1.f), 0.2f));
  world.add(make_shared<sphere>(point3(0.f, -100.5f, -1.f), 100.f));

  benchmark::start();
  renderer.render(world);
  benchmark::stop("Render");
  
  benchmark::start();
  renderer.save(image_file_name);
  benchmark::stop("Save");

  return 0;
}


