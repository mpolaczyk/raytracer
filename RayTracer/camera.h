#pragma once

#include "vec3.h"
#include "ray.h"

class camera
{
public:
  camera(float aspect_ratio, float focal_length, point3 origin)
    : aspect_ratio(aspect_ratio), focal_length(focal_length), origin(origin)
  {
    float viewport_height = 2.0f;
    float viewport_width = aspect_ratio * viewport_height;

    horizontal = vec3(viewport_width, 0, 0);
    vertical = vec3(0, viewport_height, 0);
    lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
  }

  ray inline get_ray(float u, float v) const 
  {
    return ray(origin, lower_left_corner + u * horizontal + v * vertical - origin);
  }

private:
  float aspect_ratio;
  float focal_length;
  point3 origin;
  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
};
