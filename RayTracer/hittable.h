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
  virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};


class hittable_list : public hittable 
{
public:
  hittable_list() {}
  hittable_list(shared_ptr<hittable> object) { add(object); }

  void clear() { objects.clear(); }
  void add(shared_ptr<hittable> object) { objects.push_back(object); }

  virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
  std::vector<shared_ptr<hittable>> objects;
};
