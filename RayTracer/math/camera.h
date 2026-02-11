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

class camera_config
{
public: 
  camera_config() = default;
  camera_config(const vec3& look_from, const vec3& look_dir, float field_of_view, float aspect_ratio_w, float aspect_ratio_h, float aperture, float dist_to_focus, float type = 0.0f)
    : look_from(look_from), look_dir(look_dir), field_of_view(field_of_view), aspect_ratio_w(aspect_ratio_w), aspect_ratio_h(aspect_ratio_h), aperture(aperture), dist_to_focus(dist_to_focus), type(type)
  { }

  static camera_config lerp(const camera_config& a, const camera_config& b, float f)
  {
    camera_config answer = a;
    answer.dist_to_focus = math::lerp_float(a.dist_to_focus, b.dist_to_focus, f);
    answer.type = math::lerp_float(a.type, b.type, f);
    return answer;
  }

  inline uint32_t get_hash() const
  {
    uint32_t a = hash::combine(hash::get(look_from), hash::get(look_dir), hash::get(field_of_view), hash::get(aspect_ratio_h));
    uint32_t b = hash::combine(hash::get(aspect_ratio_w), hash::get(aperture), hash::get(dist_to_focus), hash::get(type));
    return hash::combine(a, b, a, hash::get(look_dir));
  }

  // Camera movement
  void move_up(float speed)
  {
    look_from += vec3(0.0f, speed, 0.0f);
  }
  void move_down(float speed)
  {
    look_from -= vec3(0.0f, speed, 0.0f);
  }
  void move_forward(float speed)
  {
    look_from -= look_dir * speed;
  }
  void move_backward(float speed)
  {
    look_from += look_dir * speed;
  }
  void move_left(float speed)
  {
    vec3 left_dir = math::cross(look_dir, vec3(0.0f, 1.0f, 0.0f));
    look_from += left_dir * speed;
  }
  void move_right(float speed)
  {
    vec3 left_dir = math::cross(look_dir, vec3(0.0f, 1.0f, 0.0f));
    look_from -= left_dir * speed;
  }
  void rotate(float roll, float pitch)
  {
    look_dir = math::rotate_roll(look_dir, roll);  // because x is the vertical axis
    look_dir = math::rotate_pitch(look_dir, pitch);
  }

  // Persistent members
  vec3 look_from;
  vec3 look_dir;
  float field_of_view = 90.0f;
  float aspect_ratio_h = 9.0f;
  float aspect_ratio_w = 16.0f;
  float aperture = 0.0f;       // defocus blur
  float dist_to_focus = 1.0f;  // distance from camera to the focus object
  float type = 0.0f;           // 0.0f perspective camera, 1.0f orthographic camera
};

class camera
{
public:

  void configure(const camera_config* in_camera_config)
  {
    camera_conf = *in_camera_config;

    float theta = math::degrees_to_radians(camera_conf.field_of_view);
    float h = tan(theta / 2.0f);
    viewport_height = 2.0f * h;                       // viewport size at the distance 1
    viewport_width = camera_conf.aspect_ratio_w / camera_conf.aspect_ratio_h * viewport_height;

    const vec3 view_up(0.0f, 1.0f, 0.0f);
    w = math::normalize(camera_conf.look_dir);
    u = math::normalize(math::cross(view_up, w));
    v = math::cross(w, u);

    // Focus plane at origin (size of the frustum at the focus distance)
    f.horizontal = viewport_width * u * camera_conf.dist_to_focus;
    f.vertical = viewport_height * v * camera_conf.dist_to_focus;
    f.lower_left_corner = camera_conf.look_from - f.horizontal / 2.0f - f.vertical / 2.0f;

    // Camera plane at origin (proportional to type)
    c.horizontal = f.horizontal * camera_conf.type;
    c.vertical = f.vertical * camera_conf.type;
    c.lower_left_corner = camera_conf.look_from - c.horizontal / 2.0f - c.vertical / 2.0f;

    lens_radius = camera_conf.aperture / 2.0f;
  }

  ray inline get_ray(float uu, float vv) const 
  {
    vec3 rd = lens_radius * random_cache::in_unit_disk();
    vec3 offset = u * rd.x + v * rd.y;

    if (camera_conf.type == 0.0f)
    {
      // Shoot rays from the point to the focus plane - perspective camera
      vec3 fpo = f.get_point(uu, vv);                     // point on the focus plane at origin
      vec3 fpf = fpo - w * camera_conf.dist_to_focus;           // point on the focus plane at the focus distance forward camera
      return ray(camera_conf.look_from - offset, math::normalize(fpf - camera_conf.look_from + offset));
    }
    else
    {
      // Don't shoot rays from the point, shoot from the plane that is proportionally smaller to focus plane
      vec3 cpo = c.get_point(uu, vv);             // point on the camera plane at origin
      vec3 fpo = f.get_point(uu, vv);               // point on the focus plane at origin
      vec3 fpf = fpo - w * camera_conf.dist_to_focus;     // point on the plane crossing frustum, forward camera
      return ray(cpo - offset, math::normalize(fpf - cpo + offset));
    }
  }

  inline uint32_t get_hash()
  {
    return camera_conf.get_hash();
  }

  // Getters for GPU export
  inline const vec3& get_look_from() const { return camera_conf.look_from; }
  inline float get_lens_radius() const { return lens_radius; }
  inline const vec3& get_lower_left_corner() const { return f.lower_left_corner; }
  inline const vec3& get_horizontal() const { return f.horizontal; }
  inline const vec3& get_vertical() const { return f.vertical; }
  inline float get_viewport_width() const { return viewport_width; }
  inline float get_viewport_height() const { return viewport_height; }
  inline float get_dist_to_focus() const { return camera_conf.dist_to_focus; }
  inline const vec3& get_u() const { return u; }
  inline const vec3& get_v() const { return v; }
  inline const vec3& get_w() const { return w; }
  inline float get_type() const { return camera_conf.type; }

private:
  camera_config camera_conf;
  float lens_radius = 0.0f;
  float viewport_height = 2.0f;
  float viewport_width = 3.5f;
  plane f;  // focus plane at origin
  plane c;  // camera plane at origin
  vec3 w;   // back from camera vector
  vec3 u;   // right vector
  vec3 v;   // up vector
};
