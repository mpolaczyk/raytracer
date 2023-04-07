#pragma once

#include "vec3.h"

class ray
{
public:
  ray() {}
  ray(const vec3& origin, const vec3& direction)
    : origin(origin), direction(direction)
  {}

  // Returns a point at distance "t" from the origin
  inline vec3 at(float t) const
  {
    return origin + t * direction;
  }

public:
  vec3 origin;
  vec3 direction;
};
