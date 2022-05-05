// Inspired by https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include <memory>

#include "common.h"
#include "benchmark.h"
#include "frame_renderer.h"
#include "hittable.h"
#include "sphere.h"

using std::make_shared;

color3 ray_color(const ray& r, const hittable_list& world)
{
  hit_record rec;
  if (world.hit(r, 0, infinity, rec)) 
  {
    return 0.5f * (rec.normal + color3(1, 1, 1));
  }
  vec3 unit_direction = unit_vector(r.direction());
  float t = 0.5f * (unit_direction.y() + 1.0f);
  return (1.0f - t) * white + t * blue;
}

int main()
{
  const char* image_file_name = (const char*)"image.bmp";
  frame_renderer renderer = frame_renderer(1280, 720);
   
  hittable_list world;
  world.add(make_shared<sphere>(point3(0.f, 0.f, -1.f), 0.5f));
  world.add(make_shared<sphere>(point3(0.f, -100.5f, -1.f), 100.f));

  benchmark::start();
  renderer.render(&ray_color, world);
  benchmark::stop("Render");
  
  benchmark::start();
  renderer.save(image_file_name);
  benchmark::stop("Save");

  return 0;
}


