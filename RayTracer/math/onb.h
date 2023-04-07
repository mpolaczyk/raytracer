#pragma once

#include "vec3.h"

// OrthoNormal Base
struct onb
{
  onb() {};

  vec3 local(const vec3& a) const 
  {
    return a.x * u + a.y * v + a.z * w;
  }

  void build_from_w(const vec3& in_w)
  {
    w = unit_vector(in_w);
    vec3 a = (fabs(w.x) > 0.9f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    v = unit_vector(cross(w, a));
    u = cross(w, v);
  }

public:
  vec3 u, v, w;
};