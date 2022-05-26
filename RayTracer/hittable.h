#pragma once

#include <memory>
#include <vector>

#include "ray.h"

using std::shared_ptr;
using std::make_shared;

struct hit_record 
{
  point3 p;
  vec3 normal;
  float t;
};

class hittable
{
public:
  hittable() {}
  hittable(point3 cen, float r) : center(cen), radius(r) {};

  bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;

public:
  // TODO: rethink hittable type
  point3 center;
  float radius;
};



class hittable_list
{
public:
  hittable_list() {}
  hittable_list(hittable object) { add(object); }

  void clear() { objects.clear(); }
  void add(hittable object) { objects.push_back(object); }

  bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;

public:
  std::vector<hittable> objects;
};
