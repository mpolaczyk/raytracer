#pragma once

#include <limits>
#include <random>

#include "vec3.h"

const vec3 c_white = vec3(0.73f, .73f, .73f);
const vec3 c_grey = vec3(0.6f, 0.6f, 0.6f);
const vec3 c_black = vec3(0.0f, 0.0f, 0.0f);
const vec3 c_red = vec3(0.65f, 0.05f, 0.05f);
const vec3 c_green = vec3(.12f, .45f, .15f);
const vec3 c_blue = vec3(0.0f, 0.0f, 1.0f);
const vec3 c_white_blue = vec3(0.5f, 0.7f, 1.0f);
const vec3 c_yellow = vec3(1.0f, 1.0f, 0.0f);
const vec3 c_copper = vec3(0.72f, 0.45f, 0.2f);
const vec3 c_steel = vec3(0.44f, 0.47f, 0.49f);
const vec3 c_silver = vec3(0.32f, 0.34f, 0.34f);
const vec3 c_gold = vec3(1.f, 0.84f, 0.f);

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;
const float small_number = 1e-8;

float degrees_to_radians(float degrees);
float sign(float value);
vec3 random_unit_in_hemisphere(const vec3& normal);
vec3 random_in_unit_disk();
bool is_near_zero(vec3& value);
vec3 reflect(const vec3& v, const vec3& n);
vec3 refract(const vec3& uv, const vec3& n, float etai_over_etat);
float reflectance(float cosine, float ref_idx);
bool flip_normal_if_front_face(const vec3& in_ray_direction, const vec3& in_outward_normal, vec3& out_normal);
inline float lerp_float(float a, float b, float f) { return a + f * (b - a); }
inline float min1(float a, float b) { return a < b ? a : b; }
inline float max1(float a, float b) { return a < b ? b : a; }
inline float clamp(float a, float b, float f) { return  min1(b, max1(a, f)); }
inline vec3 min3(const vec3& a, const vec3& b)
{
  return vec3(min1(a[0], b[0]), min1(a[1], b[1]), min1(a[2], b[2]));
}
inline vec3 max3(const vec3& a, const vec3& b)
{
  return vec3(max1(a[0], b[0]), max1(a[1], b[1]), max1(a[2], b[2]));
}
inline void get_sphere_uv(const vec3& p, float& out_u, float& out_v)
{
  // p: a given point on the sphere of radius one, centered at the origin.
  // u: returned value [0,1] of angle around the Y axis from X=-1.
  // v: returned value [0,1] of angle from Y=-1 to Y=+1.
  //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
  //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
  //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
  float theta = acos(-p.y);
  float phi = atan2(-p.z, p.x) + pi;
  out_u = phi / (2.0f * pi);
  out_v = theta / pi;
}

namespace random_cache
{
  static int num = 500000;
  static int last_index = 0;
  static std::vector<float> cache;
  static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

  void init();
  float get_float();
  vec3 get_vec3();
}