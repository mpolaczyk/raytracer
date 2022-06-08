#pragma once

#include "vec3.h"
#include "ray.h"

class camera
{
public:
  
  camera(point3 look_from, point3 look_at, float vfov, float aspect_ratio, float alpha=0.0f)
    : aspect_ratio(aspect_ratio), alpha(alpha), origin(look_from)
  {
    float theta = degrees_to_radians(vfov);
    float h = tan(theta / 2.0f);
    viewport_height = 2.0f * h;
    viewport_width = aspect_ratio * viewport_height;

    const vec3 view_up(0.0f, 1.0f, 0.0f);
    w = unit_vector(look_from - look_at);   // back from camera vector
    u = unit_vector(cross(view_up, w));     // right vector
    v = cross(w, u);                        // up vector

    // Regular camera plane
    horizontal = viewport_width * u;
    vertical = viewport_height * v;
    lower_left_corner = origin - horizontal / 2.0f - vertical / 2.0f;

    // Proportionally smaller camera plane
    a_horizontal = viewport_width * u * alpha;
    a_vertical = viewport_height * v * alpha;
    a_lower_left_corner = origin - a_horizontal / 2.0f - a_vertical / 2.0f;
  }

  ray inline get_ray(float uu, float vv) const 
  {
    if (alpha == 0.0f)
    {
      // Shoot rays from the point to the frustum plane
      vec3 cp = lower_left_corner + uu * horizontal + vv * vertical;  // point on the camera plane
      vec3 fp = cp - w;                                               // point on the plane crossing frustum, forward camera
      return ray(origin, unit_vector(fp - origin));                   // perspective camera
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to frustum plane
      vec3 cp = a_lower_left_corner + uu * a_horizontal + vv * a_vertical;  // point on the camera plane  
      vec3 cp2 = lower_left_corner + uu * horizontal + vv * vertical;  // point on the camera plane
      float compensate = 0.9f + alpha * 0.6f;                          // objects look like they slide away, compensate for it by moving the camera forward vector
      vec3 fp = cp2 - w*compensate;                                    // point on the plane crossing frustum, forward camera
      return ray(cp, unit_vector(fp-origin));  // orthographic camera
    }
    
  }

private:
  float aspect_ratio;
  float alpha; // 0.0f perspective camera, 1.0f orthographic camera
  float viewport_height;
  float viewport_width;
  point3 origin;
  vec3 horizontal;
  vec3 vertical;
  point3 lower_left_corner;
  vec3 a_horizontal;
  vec3 a_vertical;
  point3 a_lower_left_corner;
  vec3 w;
  vec3 u;
  vec3 v;
};
