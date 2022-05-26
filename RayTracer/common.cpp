#include "common.h"

float degrees_to_radians(float degrees)
{
  return degrees * pi / 180.0f;
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