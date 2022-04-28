#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable 
{
public:
  sphere() {}
  sphere(point3 cen, float r) : center(cen), radius(r) {};

  virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
  point3 center;
  float radius;
};
