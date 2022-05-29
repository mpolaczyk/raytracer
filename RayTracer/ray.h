#pragma once

#include "vec3.h"

class ray
{
public:
  ray() {}
  ray(const point3& origin, const vec3& direction)
    : origin(origin), direction(direction)
  {}

  // Returns a point at distance "t" from the origin
  inline point3 at(float t) const
  {
    return origin + t * direction;
  }

public:
  point3 origin;
  vec3 direction;
};