#pragma once

#include "vec3.h"

class material;
class hittable;

struct hit_record
{
  vec3 p;         // hit point
  vec3 normal;
  float t;        // distance to hit point
  float u;
  float v;
  material* material_ptr = nullptr;
  bool front_face;
  hittable* object = nullptr;
};