#pragma once

#include <functional>

#include "vec3.h"
#include "bmp.h"
#include "ray.h"
#include "hittable.h"

using namespace std;
using namespace bmp;

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

  image* img = nullptr;

public:
  frame_renderer(int width, int height)
  {
    image_width = width;
    image_height = height;
    img = new image(image_width, image_height);

    // Image
    aspect_ratio = float(image_width)/ float(image_height);

    // Camera
    viewport_height = 2.0f;
    viewport_width = viewport_height * aspect_ratio;
    focal_length = 1.0f;

    cout << "Frame renderer: " << image_width << "x" << image_height << endl;
  }
  ~frame_renderer()
  {
    delete img;
  }

  void render(function<color3(const ray& r, const hittable_list& world)> ray_color, const hittable_list& world)
  {
    point3 origin = point3(0, 0, 0);
    vec3 horizontal = vec3(viewport_width, 0, 0);
    vec3 vertical = vec3(0, viewport_height, 0);
    vec3 lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
    
    for (int y = 0; y < image_height; ++y)
    {
      for (int x = 0; x < image_width; ++x)
      {
        float u = float(x) / (image_width - 1);
        float v = float(y) / (image_height - 1);
        vec3 dir = lower_left_corner + u * horizontal + v * vertical - origin;
        ray r(origin, dir);
        pixel p = pixel(ray_color(r, world));
        img->draw_pixel(x, y, &p);
      }
    }
  }

  void save(const char* file_name)
  {
    img->save_to_file(file_name);
  }
};