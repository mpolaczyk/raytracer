#pragma once

#include <cmath>
#include <limits>
#include <memory>

#include "vec3.h"

const color3 white = color3(1.0f, 1.0f, 1.0f);
const color3 blue = color3(0.5f, 0.7f, 1.0f);

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

inline float degrees_to_radians(float degrees)
{
  return degrees * pi / 180.0f;
}
