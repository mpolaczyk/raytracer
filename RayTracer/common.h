#pragma once

#include <limits>
#include <random>

#include "vec3.h"

const vec3 white = vec3(1.0f, 1.0f, 1.0f);
const vec3 grey = vec3(0.6f, 0.6f, 0.6f);
const vec3 black = vec3(0.0f, 0.0f, 0.0f);
const vec3 red = vec3(1.0f, 0.0f, 0.0f);
const vec3 green = vec3(0.0f, 1.0f, 0.0f);
const vec3 blue = vec3(0.0f, 0.0f, 1.0f);
const vec3 white_blue = vec3(0.5f, 0.7f, 1.0f);
const vec3 yellow = vec3(1.0f, 1.0f, 0.0f);

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
inline float lerp_float(float a, float b, float f) { return a + f * (b - a); }
inline float min1(float a, float b) { return a < b ? a : b; }
inline float max1(float a, float b) { return a < b ? b : a; }
inline vec3 min3(const vec3& a, const vec3& b)
{
  return vec3(min1(a[0], b[0]), min1(a[1], b[1]), min1(a[2], b[2]));
}
inline vec3 max3(const vec3& a, const vec3& b)
{
  return vec3(max1(a[0], b[0]), max1(a[1], b[1]), max1(a[2], b[2]));
}

namespace random_cache
{
  static int num = 50000;
  static int last_index = 0;
  static std::vector<float> cache;
  static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

  void init();
  float get_float();
  vec3 get_vec3();
}