#pragma once

#include <limits>
#include <random>

#include "vec3.h"

const color3 white = color3(1.0f, 1.0f, 1.0f);
const color3 blue = color3(0.5f, 0.7f, 1.0f);
const color3 black = color3(0.0f, 0.0f, 0.0f);

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

float degrees_to_radians(float degrees);

namespace random_cache
{
  static int num = 50000;
  static int last_index = 0;
  static std::vector<float> cache;
  static std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
  static std::mt19937 generator;

  void init();
  float get_float();
  vec3 get_vec3();
}