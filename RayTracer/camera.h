#pragma once

#include "vec3.h"
#include "ray.h"

struct plane
{
  vec3 horizontal;            // size horizontal
  vec3 vertical;              // size vertical
  vec3 lower_left_corner;   // world space coordinates
  vec3 get_point(float u, float v) const
  {
    return lower_left_corner + horizontal * u + v * vertical;
  }
};

struct camera_config
{
  camera_config() = default;
  camera_config(vec3 look_from, vec3 look_at, float field_of_view, float aspect_ratio, float aperture, float dist_to_focus, float type = 0.0f)
    : look_from(look_from), look_at(look_at), field_of_view(field_of_view), aspect_ratio(aspect_ratio), aperture(aperture), dist_to_focus(dist_to_focus), type(type)
  { }

  static camera_config lerp(const camera_config& a, const camera_config& b, float f)
  {
    camera_config answer = a;
    answer.dist_to_focus = lerp_float(a.dist_to_focus, b.dist_to_focus, f);
    answer.type = lerp_float(a.type, b.type, f);
    return answer;
  }

  vec3 look_from;
  vec3 look_at;
  float field_of_view = 90.0f;
  float aspect_ratio = 1.77777779f;
  float aperture = 0.0f;       // defocus blur
  float dist_to_focus = 1.0f;  // distance from camera to the focus object
  float type = 0.0f;           // 0.0f perspective camera, 1.0f orthographic camera
};

class camera
{
public:

  void set_camera(const camera_config& in_setup)
  {
    setup = in_setup;

    float theta = degrees_to_radians(setup.field_of_view);
    float h = tan(theta / 2.0f);
    viewport_height = 2.0f * h;                       // viewport size at the distance 1
    viewport_width = setup.aspect_ratio * viewport_height;

    const vec3 view_up(0.0f, 1.0f, 0.0f);
    w = unit_vector(setup.look_from - setup.look_at);   
    u = unit_vector(cross(view_up, w));     
    v = cross(w, u);                        

    // Focus plane at origin (size of the frustum at the focus distance)
    f.horizontal = viewport_width * u * setup.dist_to_focus;
    f.vertical = viewport_height * v * setup.dist_to_focus;
    f.lower_left_corner = setup.look_from - f.horizontal / 2.0f - f.vertical / 2.0f;

    // Camera plane at origin (proportional to type)
    c.horizontal = f.horizontal * setup.type;
    c.vertical = f.vertical * setup.type;
    c.lower_left_corner = setup.look_from - c.horizontal / 2.0f - c.vertical / 2.0f;

    lens_radius = setup.aperture / 2.0f;
  }

  ray inline get_ray(float uu, float vv) const 
  {
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x + v * rd.y;

    if (setup.type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      vec3 fpo = f.get_point(uu, vv);                     // point on the focus plane at origin
      vec3 fpf = fpo - w * setup.dist_to_focus;           // point on the focus plane at the focus distance forward camera
      return ray(setup.look_from - offset, unit_vector(fpf - setup.look_from + offset));
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      vec3 cpo = c.get_point(uu, vv);             // point on the camera plane at origin
      vec3 fpo = f.get_point(uu, vv);               // point on the focus plane at origin
      vec3 fpf = fpo - w * setup.dist_to_focus;     // point on the plane crossing frustum, forward camera
      return ray(cpo - offset, unit_vector(fpf - cpo + offset)); 
    }
  }

private:
  camera_config setup;
  float lens_radius = 0.0f;
  float viewport_height = 2.0f;
  float viewport_width = 3.5f;
  plane f;  // focus plane at origin
  plane c;  // camera plane at origin
  vec3 w;   // back from camera vector
  vec3 u;   // right vector
  vec3 v;   // up vector
};
