#include "stdafx.h"

#include "common.h"

float degrees_to_radians(float degrees)
{
  return degrees * pi / 180.0f;
}

float sign(float value)
{
  return value >= 0.0f ? 1.0f : -1.0f;  //  Assume 0 is positive
}

vec3 random_unit_in_hemisphere(const vec3& normal)
{
  vec3 random_unit = unit_vector(random_cache::get_vec3());
  // Invert random_unit if dot product is negative
  return sign(dot(random_unit, normal)) * random_unit;
}

bool is_near_zero(vec3& value)
{
  return (fabs(value[0]) < small_number) && (fabs(value[1]) < small_number) && (fabs(value[2]) < small_number);
}

vec3 reflect(const vec3& vec, const vec3& normal) 
{
  return vec - 2 * dot(vec, normal) * normal;
}

vec3 refract(const vec3& uv, const vec3& n, float etai_over_etat) 
{
  float cos_theta = fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

namespace random_cache
{
  void init()
  {
    for (int s = 0; s < num; s++)
    {
      cache.push_back(distribution(generator));
    }
  }

  float get_float()
  {
    last_index = (last_index + 1) % num;
    return cache[last_index];
  }

  vec3 get_vec3()
  {
    return vec3(get_float(), get_float(), get_float());
  }
}