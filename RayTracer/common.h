#pragma once

#include <limits>
#include <random>

#include "vec3.h"

const color3 white = color3(1.0f, 1.0f, 1.0f);
const color3 grey = color3(0.6f, 0.6f, 0.6f);
const color3 black = color3(0.0f, 0.0f, 0.0f);
const color3 red = color3(1.0f, 0.0f, 0.0f);
const color3 green = color3(0.0f, 1.0f, 0.0f);
const color3 blue = color3(0.0f, 0.0f, 1.0f);
const color3 white_blue = color3(0.5f, 0.7f, 1.0f);
const color3 yellow = color3(1.0f, 1.0f, 0.0f);

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