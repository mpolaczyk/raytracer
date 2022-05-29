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