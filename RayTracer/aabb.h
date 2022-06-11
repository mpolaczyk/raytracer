#pragma once

#include "ray.h"
#include "vec3.h"

class aabb 
{
public:
  aabb() = default;
  aabb(const point3& in_minimum, const point3& in_maximum)
    : minimum(in_minimum), maximum(in_maximum)
  { }
  
  bool hit(const ray& in_ray, float t_min, float t_max) const;
  bool hit2(const ray& in_ray, float t_min, float t_max) const;

  point3 minimum;
  point3 maximum;

  static aabb merge(const aabb& box0, const aabb& box1);
};
