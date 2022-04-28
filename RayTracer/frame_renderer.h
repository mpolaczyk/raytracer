#pragma once

#include <functional>

#include "vec3.h"
#include "bmp.h"
#include "ray.h"
#include "hittable.h"

using namespace std;

template<int WIDTH, int HEIGHT>
class frame_renderer
{
  // Image
  float aspect_ratio;
  int image_height;
  int image_width;

  // Camera
  float viewport_height;
  float viewport_width;
  float focal_length;

  char* image_file_name = (char*)"image.bmp";
  bmp::image<WIDTH, HEIGHT> img;

public:
  frame_renderer()
  {
    // Image
    aspect_ratio = float(WIDTH)/ float(HEIGHT);

    // Camera
    viewport_height = 2.0f;
    viewport_width = viewport_height * aspect_ratio;
    focal_length = 1.0f;

    cout << "Frame renderer: " << WIDTH << "x" << HEIGHT << endl;
  }

  void render(function<color3(const ray& r, const hittable_list& world)> ray_color, const hittable_list& world)
  {
    point3 origin = point3(0, 0, 0);
    vec3 horizontal = vec3(viewport_width, 0, 0);
    vec3 vertical = vec3(0, viewport_height, 0);
    vec3 lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
    
    for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
      {
        float u = float(x) / (WIDTH - 1);
        float v = float(y) / (HEIGHT - 1);
        vec3 dir = lower_left_corner + u * horizontal + v * vertical - origin;
        ray r(origin, dir);
        bmp::pixel p = bmp::pixel(ray_color(r, world));
        img.draw_pixel(x, y, &p);
      }
    }
  }

  void save()
  {
    img.save_to_file(image_file_name);
  }
};