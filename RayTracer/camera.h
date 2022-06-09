#pragma once

#include "vec3.h"
#include "ray.h"

struct plane
{
  vec3 horizontal;            // size horizontal
  vec3 vertical;              // size vertical
  point3 lower_left_corner;   // world space coordinates
  point3 get_point(float u, float v) const
  {
    return lower_left_corner + u * horizontal + v * vertical;
  }
};

class camera
{
public:
  
  camera(point3 look_from, point3 look_at, float field_of_view, float aspect_ratio, float aperture, float dist_to_focus, float type=0.0f)
    : type(type), origin(look_from), dist_to_focus(dist_to_focus)
  {
    float theta = degrees_to_radians(field_of_view);
    float h = tan(theta / 2.0f);
    viewport_height = 2.0f * h;                       // viewport size at the distance 1
    viewport_width = aspect_ratio * viewport_height;  

    const vec3 view_up(0.0f, 1.0f, 0.0f);
    w = unit_vector(look_from - look_at);   // back from camera vector
    u = unit_vector(cross(view_up, w));     // right vector
    v = cross(w, u);                        // up vector

    // Focus plane at origin (size of the frustum at the focus distance)
    f.horizontal = viewport_width * u * dist_to_focus;
    f.vertical = viewport_height * v * dist_to_focus;
    f.lower_left_corner = origin - f.horizontal / 2.0f - f.vertical / 2.0f;

    // Camera plane at origin (proportional to type)
    c.horizontal = f.horizontal * type;
    c.vertical = f.vertical * type;
    c.lower_left_corner = origin - c.horizontal / 2.0f - c.vertical / 2.0f;

    lens_radius = aperture / 2;
  }

  ray inline get_ray(float uu, float vv) const 
  {
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x() + v * rd.y();

    if (type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      vec3 fpo = f.get_point(uu, vv);                                   // point on the focus plane at origin
      vec3 fpf = fpo - w*dist_to_focus;                                 // point on the focus plane at the focus distance forward camera
      return ray(origin - offset, unit_vector(fpf - origin + offset));
    }
    else
    {
      // TODO: Despite shooting at the focus plane, the view is still different (I expect object in the focus plane to preserve the same size, but camera slides backwards)
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      point3 cpo = c.get_point(uu, vv);       // point on the camera plane at origin
      vec3 fpo = f.get_point(uu, vv);         // point on the focus plane at origin
      vec3 fpf = fpo - w * dist_to_focus;     // point on the plane crossing frustum, forward camera
      
      return ray(cpo - offset, unit_vector(fpf - origin + offset)); 
    }
    
  }

private:
  float lens_radius;
  float dist_to_focus;
  float type; // 0.0f perspective camera, 1.0f orthographic camera
  float viewport_height;
  float viewport_width;
  point3 origin;
  plane f;  // focus plane at origin
  plane c;  // camera plane at origin
  vec3 w;
  vec3 u;
  vec3 v;
};
